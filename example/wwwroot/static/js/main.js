(function($){"use strict";var spinner=function(){setTimeout(function(){if($('#spinner').length>0){$('#spinner').removeClass('show');}},1);};spinner();new WOW().init();$(window).scroll(function(){if($(this).scrollTop()>45){$('.navbar').addClass('sticky-top shadow-sm');}else{$('.navbar').removeClass('sticky-top shadow-sm');}});$(".navbar-nav a").on('click',function(event){if(this.hash!==""){event.preventDefault();$('html, body').animate({scrollTop:$(this.hash).offset().top-60},1500,'easeInOutExpo');if($(this).parents('.navbar-nav').length){$('.navbar-nav .active').removeClass('active');$(this).closest('a').addClass('active');}}});$(window).scroll(function(){if($(this).scrollTop()>100){$('.back-to-top').fadeIn('slow');}else{$('.back-to-top').fadeOut('slow');}});$('.back-to-top').click(function(){$('html, body').animate({scrollTop:0},1500,'easeInOutExpo');return false;});$('[data-toggle="counter-up"]').counterUp({delay:10,time:2000});$(".testimonial-carousel").owlCarousel({autoplay:true,smartSpeed:1000,margin:25,dots:false,loop:true,nav:true,navText:['<i class="bi bi-chevron-left"></i>','<i class="bi bi-chevron-right"></i>'],responsive:{0:{items:1},992:{items:2}}});})(jQuery);