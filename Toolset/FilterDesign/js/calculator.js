(function() {
  "use strict";

  var lcBandPassCalcImg = new Image();
  var lcBandPassCtx;

  function hertNotationToFloat(val) {
    val = val.toLowerCase();
    if ( val.indexOf("thz") > -1 ) {
      return parseFloat(val) * 1e12;
    } else if ( val.indexOf("ghz") > -1 ) {
      return parseFloat(val) * 1e9;
    } else if ( val.indexOf("mhz") > -1 ) {
      return parseFloat(val) * 1e6;
    } else if ( val.indexOf("khz") > -1 ) {
      return parseFloat(val) * 1e3;
    } else {
      return parseFloat(val);
    }
  }

  window.bandPassUpdate = function() {
    var f1 = hertNotationToFloat($("#bandpass_f1").val());
    var f2 = hertNotationToFloat($("#bandpass_f2").val());

    var Zo = parseFloat($("#bandpass_zo").val());

    if (!isNaN(f1) && !isNaN(f2) && !isNaN(Zo)) {
      if (f1 > f2) {
        console.error("F1 > F2");
      } else if ( f2 == f1 ) {
        console.error("F1 = F2");
      } else {
        var L1 = (Zo / (Math.PI * (f2 - f1)));
        var L2 = ((Zo * (f2 - f1)) / (4 * Math.PI * f2 * f1));
        var C1 = ((f2 - f1) / ( 4 * Math.PI * f2 * f1));
        var C2 = (1 / (Math.PI * Zo * (f2 - f1)));

        updateBandPassImage(L1, L2, C1, C2, Zo, f1, f2);
      }
    }
  };

  window.toNotationUnit = function(v) {
    var unit;
    var submultiple = ["","m","u","n","p","f","a","z","y"];
    var multiple    = ["","k","M","G","T","P","E","Z","Y"];
    var counter= 0;
    var value = v;
    if(value < 1) {
      while(value < 1) {
        counter++;
        value=value*1e3;
        if(counter==8) break;
      }
      unit = submultiple[counter];
    }else{
      while(value >= 1000) {
        counter++;
        value=value/1e3;
        if(counter==8) break;
      }
      unit = multiple[counter];
    }
    value = Math.round(value*1e2)/1e2;
    return {"unit":unit,"value":value};
  };

  window.bandPassInit = function() {
    var canvas = $("#bandpass_canvas")[0];
    lcBandPassCtx = canvas.getContext("2d");
    lcBandPassCalcImg.onload = function() {
      //updateBandPassImage(0, 0, 0, 0);
      bandPassUpdate();
    };
    lcBandPassCalcImg.src = "/img/lc_bandpass.png";
  };

  function parallelTankZ(freq, L, C) {
    var Lreact = reactance(freq, L, false);
    var Creact = reactance(freq, C, true);

    return (Lreact * Creact) / (Lreact + Creact);
  }

  function seriesTankZ(freq, L, C) {
    var Lreact = reactance(freq, L, false);
    var Creact = reactance(freq, C, true);

    return (Lreact + Creact);
  }

  function bandPassGain(freq, L1, L2, C1, C2, Zo) {
    var tR1 = Math.abs(seriesTankZ(freq, L1 / 2, 2 * C1));
    var tR2 = Math.abs(parallelTankZ(freq, L2, C2));

    var step0 = tR2 / (tR1 + tR2);
    var step1 = Zo / (tR1 + Zo);

    return step0 * step1;
  }

  function gainTodB(gain) {
    return 20 * Math.log10(gain);
  }

  function updateBandPassImage(L1, L2, C1, C2, Zo, f1, f2) {
    var L1i = {"x":60,  "y": 70 };
    var L2i = {"x":200, "y": 106};
    var C1i = {"x":118, "y": 25 };
    var C2i = {"x":130, "y": 106};
    var C3i = {"x":250, "y": 25};
    var L3i = {"x":270, "y": 70};

    var L1v = toNotationUnit(L1 / 2);
    var L2v = toNotationUnit(L2);
    var C1v = toNotationUnit(2 * C1);
    var C2v = toNotationUnit(C2);

    L1v = L1v.value + " " + L1v.unit + "H";
    L2v = L2v.value + " " + L2v.unit + "H";
    C1v = C1v.value + " " + C1v.unit + "F";
    C2v = C2v.value + " " + C2v.unit + "F";

    lcBandPassCtx.clearRect(0,0,350,160);
    lcBandPassCtx.drawImage(lcBandPassCalcImg, 0, 0);
    lcBandPassCtx.font = "16px Arial";

    //  Draw L1
    var t = lcBandPassCtx.measureText(L1v);
    lcBandPassCtx.fillText(L1v, L1i.x - t.width / 2, L1i.y);
    //  Draw L2
    t = lcBandPassCtx.measureText(L2v);
    lcBandPassCtx.fillText(L2v, L2i.x, L2i.y);
    //  Draw L3
    t = lcBandPassCtx.measureText(L1v);
    lcBandPassCtx.fillText(L1v, L3i.x, L3i.y);

    //  Draw C1
    t = lcBandPassCtx.measureText(C1v);
    lcBandPassCtx.fillText(C1v, C1i.x - t.width / 2, C1i.y);
    //  Draw C2
    t = lcBandPassCtx.measureText(C2v);
    lcBandPassCtx.fillText(C2v, C2i.x - t.width , C2i.y);
    //  Draw C3
    t = lcBandPassCtx.measureText(C1v);
    lcBandPassCtx.fillText(C1v, C3i.x - t.width , C3i.y);

    updateBandPassResponse(L1, L2, C1, C2, Zo, f1, f2);
  }

  function reactance(freq, val, capacitive) {
    capacitive = capacitive || false;
    var twopiv = 2 * Math.PI * freq * val;
    return capacitive ? -1 / twopiv : twopiv;
  }

  function updateBandPassResponse(L1, L2, C1, C2, Zo, f1, f2) {
    var canvas = $("#bandpass_response")[0];
    var ctx = canvas.getContext("2d");

    var dF = f2 - f1;
    f1 -= dF * 2;
    f2 += dF * 2;

    if (f1 < 0)
      f1 = 1;

    var origin = { x: 40, y: 20 };
    var marksS = 60;
    var marksY = 30;
    var marks = { x: canvas.width / marksS, y: canvas.height / marksY };

    var vals = [];
    var deltaF = (f2 - f1) / canvas.width;

    ctx.strokeStyle="#000000";
    var minGain = 1;
    var maxGain = -1000;

    for (var x=0; x<canvas.width; x++) {
      var gain = bandPassGain(f1 + x * deltaF, L1, L2, C1, C2, Zo);
      gain = gainTodB(gain);
      vals.push(gain);
      minGain = Math.min(minGain, gain);
      maxGain = Math.max(maxGain, gain);
    }

    function scaledB(dB) {
      return (canvas.height - origin.y - 10) * ((dB - minGain) / (maxGain - minGain));
    }

    function reverseScaledB(y) {
      return (y / (canvas.height - origin.y - 10)) * (maxGain - minGain) + minGain;
    }

    ctx.clearRect(0,0,canvas.width,canvas.height);

    // Draw Graph base

    ctx.save();
    ctx.translate(0, canvas.height);
    ctx.scale(1, -1);

    ctx.beginPath();

    ctx.moveTo(origin.x, origin.y);
    ctx.lineTo(origin.x + canvas.width, origin.y + 0);
    ctx.moveTo(origin.x, origin.y);
    ctx.lineTo(origin.x + 0, origin.y + canvas.height);

    for (x=1; x<marks.x; x++) {
      ctx.moveTo(origin.x + marksS * x, origin.y - 5);
      ctx.lineTo(origin.x + marksS * x, origin.y + 5);
    }

    for (var y=1; y<marks.y; y++) {
      ctx.moveTo(origin.x - 5, origin.y + marksY * y);
      ctx.lineTo(origin.x + 5, origin.y + marksY * y);
    }

    ctx.stroke();

    // Draw Graph Grid
    ctx.strokeStyle="#AAAAAA";

    ctx.beginPath();

    for (x=1; x<marks.x; x++) {
      ctx.moveTo(origin.x + marksS * x, origin.y);
      ctx.lineTo(origin.x + marksS * x, canvas.height);
    }

    for (y=1; y<marks.y; y++) {
      ctx.moveTo(origin.x, origin.y + marksY * y);
      ctx.lineTo(origin.x + canvas.width, origin.y + marksY * y);
    }

    ctx.stroke();

    // Draw Response Curve

    ctx.beginPath();
    ctx.strokeStyle="#FF0000";
    ctx.moveTo(origin.x, origin.y + scaledB(vals[0]));

    for (x=0;x<canvas.width;x++) {
      y = origin.y + scaledB(vals[x]);
      if (y < origin.y)
        ctx.moveTo(origin.x + x, y);
      else
        ctx.lineTo(origin.x + x, y);
    }

    ctx.stroke();

    ctx.restore();

    // Draw Texts

    ctx.font = "10px Arial";

    for (x=0; x<marks.x; x++) {
      var f = toNotationUnit(f1 + (marksS * x)  * deltaF);
      f = f.value + f.unit;
      var t = ctx.measureText(f);
      ctx.fillText(f, origin.x + marksS * x - t.width / 2, canvas.height - origin.y + 16);
    }

    for (y=0; y<marks.y; y++) {
      var yV = canvas.height - (origin.y + marksY * y - 3);
      var d = Math.round(reverseScaledB(canvas.height - yV)) + " dB";
      var ty = ctx.measureText(d);
      ctx.fillText(d, origin.x - ty.width - 8, yV);
    }

  }

})();
