$(document).ready(function () {
    /**
     * Handle video jump button press
     */
    $('.btn-video-jump').on('click', function () {
        const sTimeToJump = $(this).attr('data-time');
        const iframe = document.querySelector('iframe');
        const player = new Vimeo.Player(iframe);

        player.setCurrentTime(sTimeToJump);
    });
});