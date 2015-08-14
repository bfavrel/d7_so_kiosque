<?php

/**
 * Informs so_kiosque about new actors' types.
 *
 * @return array : indexed by actors' types machine name :
 *                      - 'label' : string : human readable name of type.
 */
function hook_kiosque_actor_type() {}

/**
 * Provides actor's configuration forms to so_kiosque.
 *
 * @param string $type : actor's type.
 * @param array $configuration : module's configuration of the actor.
 * @param string $context : which level of settings ('casting' or 'sequence') to provide form for.
 * @param boolean $touch : does the form have to be builded ? (see @return below)
 *
 * @return : elements array if $touch is set to false, or a boolean which indicates if a form is needed for the provided context.
 */
function hook_kiosque_actor_configure($type, $configuration, $context, $touch = false) {}

/**
 * Provides content to kiosque.
 *
 * @param array &$content
 * @param string &$title : page title
 * @param string $type : actor's type.
 * @param array $configuration : module's configuration of the actor.
 * @param array &$params : kiosque's internal settings (@see so_kiosque.module:so_kiosque_init_sequence_display() for details)
 * @param array &$storage : modules' data storage (@see so_kiosque.module:so_kiosque_init_sequence_display() for details)
 */
function hook_kiosque_actor_content(&$content, &$title, $type, $configuration, &$params, &$storage) {}