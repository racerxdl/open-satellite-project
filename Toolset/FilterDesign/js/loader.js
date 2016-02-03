(function() {
  "use strict";

  function refreshDOM() {
    componentHandler.upgradeDom();
  }

  window.loadPage = function(page) {
    switch(page) {
      case "lcbandpass":
        $(".mdl-layout__content").load("filters/lcbandpass.html", refreshDOM);
      break;
    }
    $(".mdl-layout__drawer-button").click();
  };

  $(document).ready(function() {
    loadPage('lcbandpass');
  });
})();