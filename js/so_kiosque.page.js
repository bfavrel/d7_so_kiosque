(function ($) {

    window.KiosqueSlideshow = function(options) {
        this._init(options);
    }

    KiosqueSlideshow.prototype = {

        _defaults: {

            mainDelay: 10000,
            retryDelay: 1000,

            showContent: function(timeout) {},

            replaceContent: function(content) {
                document.write(content);
                document.close();
            }
        },

        _init: function(options) {

            this.settings = $.extend({}, this._defaults, options);
            this.content = "";

            this.settings.showContent(this.settings.mainDelay);
            this._getNextContent();
            this._launchCountdown(this.settings.mainDelay);
        },

        _getNextContent: function() {

            $.ajax({
                url: Drupal.settings.basePath + 'kiosque/ajax/run',
                context: this,
                success: function(data) {
                    this.content = data;
                }
            });
        },

        _launchCountdown: function(delay) {
            setTimeout(this._countdownExpired.bind(this), delay);
        },

        _countdownExpired: function() {

            if(this.content != "") {
                this.settings.replaceContent(this.content);
            } else {
                this._launchCountdown(this.settings.retryDelay);
            }
        },
    };

})(jQuery);