$(document).ready(function() {
    $('#scrollToSteps').on('click', function (oEvent) {   
        oEvent.preventDefault();
        $('html,body').animate({
            scrollTop: $("#steps").offset().top
        }, 400);
    });
   
   
// document ready  
});