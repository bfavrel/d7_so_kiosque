<?php

// ----- SEQUENCES

function so_kiosque_sequences_form($form, &$form_state) {

    $form = array(
        '#tree' => true,

        '#theme_wrappers' => array('fieldset', 'form'),
        '#title' => t("Sequences"),

        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_kiosque') . '/css/so_kiosque_admin.css')
        ),

        'sequences' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_sequences',
        ),

        'add_sequence' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_sequences_add',

            '##locked' => count(so_kiosque_get_actor_definition()) == 0,

            'name' => array(
                '#type' => 'textfield',
                '#title' => t("New sequence name"),
                '#required' => true,
            ),

            'add' => array(
                '#type' => 'submit',
                '#value' => t("Add"),
            ),
        ),
    );

    $sequences = so_kiosque_get_sequence_definition();

    foreach($sequences as $sequence) {

        $form['sequences'][] = array(

            'name' => array('#markup' => $sequence->name),

            'operations' => array(
                'actors' => array('#markup' => l(t("Actors"), 'admin/structure/kiosque/sequence_actors/' . $sequence->id)),
                'params' => array('#markup' => l(t("Parameters"), 'admin/structure/kiosque/sequence_params/' . $sequence->id)),
                'delete' => array('#markup' => l(t("Delete"), 'admin/structure/kiosque/sequence_delete/' . $sequence->id)),
            ),
        );
    }

    return $form;
}

function theme_kiosque_sequences($vars) {

    $output = "";

    $element = $vars['element'];

    $sequences = element_children($element);

    if(empty($sequences)) {
        return "<div class='overview_empty_msg'>- " . t("No sequence set yet") . " -</div>";
    }

    $rows = array();

    foreach($sequences as $sequence) {
        $rows[] = array(
            render($element[$sequence]['name']),

            render($element[$sequence]['operations']['actors']) . " - " .
            render($element[$sequence]['operations']['params']) . " - " .
            render($element[$sequence]['operations']['delete']),
        );
    }

    $headers = array(
        t("Name"),
        t("Operations"),
    );

    $output .= theme('table', array('header' => $headers, 'rows' => $rows));

    return $output;
}

function theme_kiosque_sequences_add($vars) {
    $output = "";

    $element = $vars['element'];

    if($element['##locked'] == true) {
        return "<div class='overview_empty_msg'>- " . t("Some actors have to be defined first") . " -</div>";
    }

    $rows = array(
        array(
            render($element['name']),
            render($element['add']),
        ),
    );

    $output .= theme('table', array('header' => null, 'rows' => $rows));

    return $output;
}

function so_kiosque_sequences_form_submit($form, &$form_state) {

    $record = array(
        'id' => null,
        'name' => $form_state['values']['add_sequence']['name'],
        'params' => serialize(array()),
    );

    drupal_write_record('kiosque_sequences', $record);

    drupal_set_message(t("Sequence has been created.<br />The next step consists in adding actors to the sequence."));
    drupal_goto('admin/structure/kiosque/sequence_actors/' . $record['id']);
}

function so_kiosque_sequence_params_form($form, &$form_state, $id_sequence) {

    $definition = so_kiosque_get_sequence_definition($id_sequence);
    drupal_set_title(t("Sequence '@sequence'", array('@sequence' => $definition->name)));

    $form = array(
        '#theme_wrappers' => array('fieldset', 'form'),
        '#title' => t("Edit sequence"),

        'name' => array(
            '#type' => 'textfield',
            '#title' => t("Name"),
            '#default_value' => $definition->name,
            '#required' => true,
        ),

        'save' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),

        'back' => array(
            '#markup' => l(t("Back"), 'admin/structure/kiosque/sequences'),
        ),
    );

    return $form;
}

function so_kiosque_sequence_params_form_submit($form, &$form_state) {

    $record = array(
        'id' => $form_state['build_info']['args'][0],
        'name' => $form_state['values']['name'],
        'params' => serialize(array()),
    );

    drupal_write_record('kiosque_sequences', $record, 'id');

    drupal_set_message(t("Sequence has been updated"));
    drupal_goto('admin/structure/kiosque/sequences');
}

function so_kiosque_sequence_delete_form($form, $form_state, $id_sequence) {

    $definition = so_kiosque_get_sequence_definition($id_sequence);

    return confirm_form(
        $form,
        t("Are you sure you want to delete the sequence '@sequence' ?", array('@sequence' => $definition->name)),
        'admin/structure/kiosque/sequences',
        null,
        t("Delete")
    );
}

function so_kiosque_sequence_delete_form_submit($form, &$form_state) {

    $definition = so_kiosque_get_sequence_definition($form_state['build_info']['args'][0]);

    $req = "DELETE FROM {kiosque_sequences} WHERE id=?";
    db_query($req, array($definition->id));

    $req = "DELETE FROM {kiosque_sequences_actors} WHERE id_sequence=?";
    db_query($req, array($definition->id));

    drupal_set_message(t("Sequence '@sequence' has been deleted", array('@sequence' => $definition->name)), 'status');
    drupal_goto('admin/structure/kiosque/sequences');
}

// ----- SEQUENCE ACTORS

function so_kiosque_sequence_actors_form($form, &$form_state, $id_sequence) {

    $settings = variable_get('so_kiosque_settings', array());

    $definition = so_kiosque_get_sequence_definition($id_sequence);
    drupal_set_title(t("Sequence '@sequence'", array('@sequence' => $definition->name)));

    // tous les acteurs qui ne sont pas déjà présents dans la séquence
    $results = db_query(
        "SELECT ka.id, ka.name FROM {kiosque_actors} AS ka
        LEFT JOIN {kiosque_sequences_actors} AS ksa ON ksa.id_actor=ka.id AND ksa.id_sequence=?
        WHERE ksa.id_actor IS NULL
        ORDER BY ka.name ASC",
        array($id_sequence)
    );

    $actor_options = array();

    while($row = $results->fetchObject()) {
        $actor_options[$row->id] = $row->name;
    }

    $form = array(
        '#tree' => true,

        '#theme_wrappers' => array('fieldset', 'form'),
        '#title' => t("Actors"),

        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_kiosque') . '/css/so_kiosque_admin.css')
        ),

        'actors' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_sequence_actors',
        ),

        'save' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),

        'add_actor' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_sequence_actors_add',

            'actor' => array(
                '#type' => 'select',
                '#title' => t("Available actors"),
                '#options' => $actor_options,
            ),

            'add' => array(
                '#type' => 'submit',
                '#value' => t("Add"),
                '#submit' => array('so_kiosque_sequence_actors_form_add_submit'),
            ),
        ),

        'back' => array(
            '#markup' => l(t("Back"), 'admin/structure/kiosque/sequences'),
        ),
    );

    $actors = so_kiosque_get_sequence_actors($id_sequence, null, 'weight');

    if(empty($actors)) {
        unset($form['save']);
    }

    $delta = 50;
    $weight = -$delta;

    foreach($actors as $actor) {

        $form['actors'][] = array(

            'id_actor' => array(
                '#type' => 'value',
                '#value' => $actor->id,
            ),

            'active' => array(
                '#type' => 'checkbox',
                '#title' => null,
                '#default_value' => $actor->active,
            ),

            'name' => array(
                '#markup' => $actor->name,
            ),

            'weight' => array(
                '#type' => 'weight',
                '#title' => null,
                '#delta' => $delta,
                '#default_value' => $weight++,
            ),

            'operations' => array(
                'delete' => array('#markup' => l(t("Delete"), 'admin/structure/kiosque/sequence_actor_delete/' . $id_sequence .  '/' . $actor->id)),
                'display' => $settings['development'] == true ? array('#markup' => l(t("Development display"), 'kiosque/development/' . $id_sequence .  '/' . $actor->id, array('attributes' => array('target' => '_blank')))) : array(),
            ),
        );
    }

    return $form;
}

function theme_kiosque_sequence_actors($vars) {
    $output = "";

    $element = $vars['element'];

    $actors = element_children($element);

    if(empty($actors)) {
        return "<div class='overview_empty_msg'>- " . t("No actor in this sequence yet") . " -</div>";
    }

    $rows = array();

    foreach($actors as $actor) {

        $element[$actor]['weight']['#attributes']['class'] = array('draggable_weight');

        $rows[] = array(
            'data' => array(
                render($element[$actor]['active']),
                render($element[$actor]['name']),
                render($element[$actor]['weight']),
                render($element[$actor]['operations']['delete']) .
                (array_key_exists('#markup', $element[$actor]['operations']['display']) ? " - " . render($element[$actor]['operations']['display']) : ""),
            ),
            'class' => array('draggable'),
        );
    }

    $output .= theme('table', array(
        'header' => array(
            t("Active"),
            t("Name"),
            t("Weight"),
            t("Operations"),
        ),
        'rows' => $rows,
        'attributes' => array('id' => 'so-kiosque-sequence-actors'),
    ));

    drupal_add_tabledrag('so-kiosque-sequence-actors', 'order', 'sibling', 'draggable_weight');

    return $output;
}

function theme_kiosque_sequence_actors_add($vars) {
    $output = "";

    $element = $vars['element'];

    $rows = array(
        array(
            render($element['actor']),
            render($element['add']),
        ),
    );

    $output .= theme('table', array('header' => null, 'rows' => $rows));

    return $output;
}

function so_kiosque_sequence_actors_form_submit($form, &$form_state) {

    foreach($form_state['values']['actors'] as $actor) {
        db_query(
            "UPDATE {kiosque_sequences_actors}
            SET active=?, weight=?
            WHERE id_actor=? AND id_sequence=?",
            array(
                $actor['active'],
                $actor['weight'],
                $actor['id_actor'],
                $form_state['build_info']['args'][0],
            )
        );
    }
    
    drupal_set_message(t("Actors have been saved."));
}

function so_kiosque_sequence_actors_form_add_submit($form, &$form_state) {

    $record = array(
        'id_actor' => $form_state['values']['add_actor']['actor'],
        'id_sequence' => $form_state['build_info']['args'][0],
        'active' => 0,
        'weight' => 0,
        'params' => serialize(array()),
    );

    drupal_write_record('kiosque_sequences_actors', $record);

    drupal_set_message(t("Actor has been added to the sequence."));
    $form_state['rebuild'] = true;
}

function so_kiosque_sequence_actor_delete($id_sequence, $id_actor) {

    db_query("DELETE FROM {kiosque_sequences_actors} WHERE id_actor=? AND id_sequence=?", array($id_actor, $id_sequence));

    drupal_set_message(t("Actor has been removed"), 'status');
    drupal_goto('admin/structure/kiosque/sequence_actors/' . $id_sequence);
}

// ----- CASTING

function so_kiosque_actors_form($form, &$form_state) {

    $modules = module_implements('kiosque_actor_type');

    $actor_types = array();

    foreach($modules as $module) {
        $types = module_invoke($module, 'kiosque_actor_type');

        foreach($types as $type => $infos) {
            $actor_types[$module . ':' . $type] = $infos['label'];
        }
    }

    $form = array(
        '#tree' => true,

        '#theme_wrappers' => array('fieldset', 'form'),
        '#title' => t("Actors"),

        '#attached' => array(
            'css' => array(drupal_get_path('module', 'so_kiosque') . '/css/so_kiosque_admin.css')
        ),

        'actors' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_actors',
        ),

        'add_actor' => array(
            '#type' => 'container',
            '#theme' => 'kiosque_actors_add',

            'name' => array(
                '#type' => 'textfield',
                '#title' => t("New actor name"),
            ),

            'type' => array(
                '#type' => 'select',
                '#title' => t("Actor type"),
                '#options' => $actor_types,
            ),

            'add' => array(
                '#type' => 'submit',
                '#value' => t("Add"),
            ),
        ),
    );

    $actors = so_kiosque_get_actor_definition();

    foreach($actors as $actor) {

        $actor_type = explode(':', $actor->type); // namespace(module):type

        $has_config = count(array_filter(module_invoke_all('kiosque_actor_configure', $actor_type[1], $actor->params, 'casting', true))) > 0;

        $form['actors'][] = array(

            'name' => array('#markup' => $actor->name),

            'type' => array('#markup' => $actor_types[$actor->type]),

            'operations' => array(
                'params' => $has_config == true ? array('#markup' => l(t("Parameters"), 'admin/structure/kiosque/actor_params/' . $actor->id, array('query' => drupal_get_destination()))) : array(),
                'delete' => array('#markup' => l(t("Delete"), 'admin/structure/kiosque/actor_delete/' . $actor->id, array('query' => drupal_get_destination()))),
            ),
        );
    }

    return $form;
}

function theme_kiosque_actors($vars) {
    $output = "";

    $element = $vars['element'];

    $actors = element_children($element);

    if(empty($actors)) {
        return "<div class='overview_empty_msg'>- " . t("No actor set yet") . " -</div>";
    }

    $rows = array();

    foreach($actors as $actor) {
        $rows[] = array(
            render($element[$actor]['name']),
            render($element[$actor]['type']),

            render($element[$actor]['operations']['params']) .
            (array_key_exists('#markup', $element[$actor]['operations']['params']) ? " - " : "") .
            render($element[$actor]['operations']['delete']),
        );
    }

    $headers = array(
        t("Name"),
        t("Type"),
        t("Operations"),
    );

    $output .= theme('table', array('header' => $headers, 'rows' => $rows));

    return $output;
}

function theme_kiosque_actors_add($vars) {
    $output = "";

    $element = $vars['element'];

    $rows = array(
        array(
            render($element['name']),
            render($element['type']),
            render($element['add']),
        ),
    );

    $output .= theme('table', array('header' => null, 'rows' => $rows));

    return $output;
}

function so_kiosque_actors_form_submit($form, &$form_state) {

    $record = array(
        'id' => null,
        'name' => $form_state['values']['add_actor']['name'],
        'type' => $form_state['values']['add_actor']['type'],
        'active' => 0,
        'params' => serialize(array()),
    );

    drupal_write_record('kiosque_actors', $record);

    drupal_set_message(t("Actor has been created.<br />It must be configured now."));
    drupal_goto('admin/structure/kiosque/actor_params/' . $record['id']);
}

function so_kiosque_actor_params_form($form, &$form_state, $id_actor) {

    $definition = so_kiosque_get_actor_definition($id_actor);
    drupal_set_title(t("Actor '@actor'", array('@actor' => $definition->name)));

    $actor_type = explode(':', $definition->type); // namespace(module):type

    $modules = module_implements('kiosque_actor_configure');

    $configurations = array(
        '#type' => 'container',
    );

    foreach($modules as $module) {
        $elements = module_invoke($module, 'kiosque_actor_configure', $actor_type[1], $definition->params['modules'][$module], 'casting');

        if(!empty($elements)) {
            $configurations[$module] = $elements;
        }
    }

    $form = array(
        '#tree' => true,

        'modules' => $configurations,

        'save' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),

        'back' => array(
            '#markup' => l(t("Back"), 'admin/structure/kiosque/actors'),
        ),
    );

    return $form;
}

function so_kiosque_actor_params_form_submit($form, &$form_state) {

    $record = so_kiosque_get_actor_definition($form_state['build_info']['args'][0]);
    $record->params['modules'] = $form_state['values']['modules'];
    $record->params = serialize($record->params);

    drupal_write_record('kiosque_actors', $record, 'id');

    drupal_set_message(t("Actor's params have been saved"));
}

function so_kiosque_actor_delete_form($form, $form_state, $id_actor) {

    $definition = so_kiosque_get_actor_definition($id_actor);

    return confirm_form(
        $form,
        t("Are you sure you want to delete the actor '@actor' ?", array('@actor' => $definition->name)),
        null,
        null,
        t("Delete")
    );
}

function so_kiosque_actor_delete_form_submit($form, &$form_state) {

    $definition = so_kiosque_get_actor_definition($form_state['build_info']['args'][0]);

    $req = "DELETE FROM {kiosque_actors} WHERE id=?";
    db_query($req, array($definition->id));

    $req = "DELETE FROM {kiosque_sequences_actors} WHERE id_actor=?";
    db_query($req, array($definition->id));

    drupal_set_message(t("Actor '@actor' has been deleted", array('@actor' => $definition->name)), 'status');
    drupal_goto();
}

// ----- MODULE SETTINGS

function so_kiosque_settings_form($form, &$form_state) {

    $settings = variable_get('so_kiosque_settings', array(
        'theme' => '',
        'min_delay' => 10000,
        'development' => false
    ));

    $themes = list_themes();

    $theme_options = array();

    foreach($themes as $theme) {
        if($theme->status == 0) {continue;}

        $theme_options[$theme->name] = $theme->info['name'];
    }

    $form = array(
        'theme' => array(
            '#type' => 'select',
            '#title' => t("Theme to use for displaying the kiosque"),
            '#options' => $theme_options,
            '#default_value' => $settings['theme'],
        ),

        'min_delay' => array(
            '#type' => 'textfield',
            '#title' => t("Minimum display time for a page"),
            '#description' => t("in milliseconds"),
            '#default_value' => $settings['min_delay'],
            '#size' => 6,
            '#required' => true,
        ),

        'development' => array(
            '#type' => 'checkbox',
            '#title' => t("Activate development mode"),
            '#description' => t("Provides menu callbacks to display each actors of each sequences in the kiosque's theme, and inhibits built-in slideshow."),
            '#default_value' => $settings['development'],
        ),

        'save' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),
    );

    return $form;
}

function so_kiosque_settings_form_submit($form, &$form_state) {

    $settings = array(
        'theme' => $form_state['values']['theme'],
        'min_delay' => $form_state['values']['min_delay'],
        'development' => $form_state['values']['development'],
    );

    variable_set('so_kiosque_settings', $settings);

    drupal_set_message(t("Settings have been saved."));
}