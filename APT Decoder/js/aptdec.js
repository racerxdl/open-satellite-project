/**
  OpenSatelliteProject - A OpenSource Satellite Study
  Copyright (C) 2015  Lucas Teske < lucas at teske dot br dot net >

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *  The Sync A signal is a 7 cycle 1040Hz sine inside each line.
 *  This array represents the 1040Hz signal in 11025 Sample Rate
 */
var syncData = new Float32Array([
   0.000000000,  0.557489439,  0.925637660,  0.979409768,  0.700543038,
   0.183749518, -0.395451207, -0.840344072, -0.999829250, -0.819740483,
  -0.361241666,  0.219946358,  0.726433574,  0.986200747,  0.911022649,
   0.526432163, -0.036951499, -0.587785252, -0.938988361, -0.971281032,
  -0.673695644, -0.147301698,  0.429120609,  0.859799851,  0.998463604,
   0.798017227,  0.326538713, -0.255842778, -0.751331890, -0.991644696,
  -0.895163291, -0.494655843,  0.073852527,  0.617278221,  0.951056516,
   0.961825643,  0.645928062,  0.110652682, -0.462203884, -0.878081248,
  -0.995734176, -0.775203976, -0.291389747,  0.291389747,  0.775203976,
   0.995734176,  0.878081248,  0.462203884, -0.110652682, -0.645928062,
  -0.961825643, -0.951056516, -0.617278221, -0.073852527,  0.494655843,
   0.895163291,  0.991644696,  0.751331890,  0.255842778, -0.326538713,
  -0.798017227, -0.998463604, -0.859799851, -0.429120609,  0.147301698,
   0.673695644,  0.971281032,  0.938988361,  0.587785252,  0.036951499,
  -0.526432163, -0.911022649, -0.986200747, -0.726433574, -0.219946358
]);

/**
 *  Low Pass FIR Filter Coefficients.
 *  Calculated with 11025Hz Sample Rate, 1200Hz Half Amplitude Frequency, 50 taps
 */
var lowPass = new Float32Array([
-0.000016540625438210554, -0.00004612725751940161, -0.000003658998139144387,  0.00020216309349052608,
 0.000544912938494235300,  0.00079004385042935610,  0.000547181000001728500, -0.00048261866322718560,
-0.002150485524907708000, -0.00363161996938288200, -0.003605136647820472700, -0.00095574476290494200,
 0.004239066969603300000,  0.01002142205834388700,  0.012828110717236996000,  0.00902324449270963700,
-0.002744758035987615600, -0.01950957253575325000, -0.033640831708908080000, -0.034976765513420105,
-0.015012636780738830000,  0.02854425460100174000,  0.089116714894771580000,  0.15218372642993927000,
 0.199898496270179750000,  0.21767434477806090000,  0.199898496270179750000,  0.15218372642993927000,
 0.089116714894771580000,  0.02854425460100174000, -0.015012636780738830000, -0.03497676551342010500,
-0.033640831708908080000, -0.01950957253575325000, -0.002744758035987615600,  0.00902324449270963700,
 0.012828110717236996000,  0.01002142205834388700,  0.004239066969603300000, -0.00095574476290494200,
-0.003605136647820472700, -0.00363161996938288200, -0.002150485524907708000, -0.00048261866322718560,
 0.000547181000001728500,  0.00079004385042935610,  0.000544912938494235300,  0.00020216309349052608,
-0.000003658998139144387, -0.00004612725751940161, -0.000016540625438210554
]);

/*
 *  Extends the Uint8Array to be able to be converted to a string
 */
Uint8Array.prototype.asString = function() {
  var o = "";
  for(var i=0;i<this.byteLength;i++)
      o += String.fromCharCode(this[i]);
  return o;
};

/*
 *  Puts a string inside the UInt8Array
 */
Uint8Array.prototype.putString = function(offset, string) {
  if (string === undefined) {
    string = offset;
    offset = 0;
  }
  for (var i=0;i<string.length;i++) {
    this[offset+i] = string.charCodeAt(i);
  }
  return offset+string.length;
};

function fir(v1, v2) {
  for(var i=0;i<v2.length;i++) {
    var acc = 0;
    for(var x=0;x<v1.length;x++) {
      if (i+x >= v2.length)
        break;
      acc += v2[i+x] * v1[x];
    }
    v2[i] = acc;
  }
}

function interpolate(data, x) {
  var x0 = Math.floor(x);
  var x1 = x0 + 1;

  if (x0 >= data.length)
    return 0;

  if (x0 == x || x1 >= data.length)
    return data[x0];

  var y0 = data[x0];
  var y1 = data[x1];

  return y0 + ( (x - x0) / (x1 - x0) ) * (y1 - y0);
}

function resample(input, interpolation, decimation, coeffs) {
  var factor = interpolation / decimation;
  var output = new Float32Array( factor * input.length );
  var nInput = input.slice(0,input.length);

  fir(coeffs, nInput);

  for (var i=0;i<output.length;i++) {
      output[i] = nInput[Math.floor(i / factor)];
  }

  return output;
}

function normalize(data, sampleSize) {
  var max = -100000;
  var min = 100000;
  for(var i=0;i<sampleSize;i++) {
    max = Math.max(data[i],max);
    min = Math.min(data[i],min);
  }

  if(max !== 0) {
    for(i=0;i<sampleSize;i++)
      data[i] = (data[i]-min) / (max-min);
  }
  return data;
}

function mean(data, sampleSize) {
  var sMean = 0;
  for (var i=0;i<sampleSize;i++) {
    if (isNaN(data[i]))
      data[i] = 0;
    sMean += data[i];
  }
  return sMean / sampleSize;
}

function getSync(start, range, dataMean, data) {
  var maxVal = 0;
  var maxIndex = 0;
  for(var i = start; i < start+range; i++) {
    sum = 0;
    for(var c = 0; c < syncData.length; c++) {
      sum += (data[i+c]-dataMean)*syncData[c];
    }
    if(sum>maxVal){
      maxVal = sum;
      maxIndex = i;
    }
  }

  return {"index":maxIndex,"score":maxVal};
}

function clamp(num, min, max) {
  return num < min ? min : num > max ? max : num;
}

var xhr = new XMLHttpRequest();
xhr.open('GET', 'N18_4827.wav', true);
xhr.responseType = 'arraybuffer';

xhr.onload = function(e) {
  if (this.status == 200) {
    var buff = e.target.response;
    var u8a = new Uint8Array(buff);
    var view = new DataView(buff);

    //  RIFF Chunk
    var chunkId = u8a.slice(0,4).asString();
    var chunkSize = view.getUint32(4, true);
    var chunkFormat = u8a.slice(8,12).asString();
    console.log("Chunk ID: "+chunkId);
    console.log("Chunk Size: "+chunkSize);
    console.log("Chunk Format: "+chunkFormat);

    //  Format Chunk
    var subChunk1Id = u8a.slice(12,16).asString();
    var subChunk1Size = view.getUint32(16, true);
    var audioFormat = view.getUint16(20, true);
    var numChannels = view.getUint16(22, true);
    var sampleRate = view.getUint32(24, true);
    var byteRate = view.getUint32(28, true);
    var blockAlign = view.getUint16(32, true);
    var bitsPerSample = view.getUint16(34, true);

    console.log("Subchunk1 Size: "+subChunk1Size);
    console.log("Audio Format: "+audioFormat);
    console.log("Number of Channels: "+numChannels);
    console.log("Sample Rate: "+sampleRate);
    console.log("Byte Rate: "+byteRate);
    console.log("Block Align: "+blockAlign);
    console.log("Bits per Sample: "+bitsPerSample);

    // Data Chunk
    var subChunk2ID = u8a.slice(36,40).asString();
    var subChunk2Size = view.getUint32(40, true);
    var data = buff.slice(44, subChunk2Size);
    console.log("Subchunk2 ID: "+subChunk2ID);
    console.log("Subchunk2 Size: "+subChunk2Size);

    /**
     *  Lets rectify the sinal and rescale to 0 to 1
     */
    var fData = new Float32Array(data.byteLength / blockAlign);
    data = new Int16Array(data);
    for(var i=0;i<data.length;i++) {
      fData[i] = Math.abs(data[i]) / 32768;
    }

    /**
     *  Now lets filter out the sinal for high frequency noises.
     */
    console.log("Filtering");
    fir(lowPass, fData);

    /**
     *  And normalize it.
     */
    console.log("Normalizing");
    normalize(fData, fData.length);

    /**
     *  The APT Frame has a fixed line width of 2080 (Syncs + Telemetry + Frames)
     *  that is sent in a baudrate of 4160 bits, so since our signal is 11025Hz of
     *  of sample rate, we will have 2080 * ( 11025 / 4160 ) samples per line.
     */
    var samplesPerLine = 2080 * (11025 / 4160);

    console.log("Constructing image");
    var numColumns = 2080;
    var numLines = data.length / samplesPerLine;

    console.log("Number of lines: "+numLines);

    var canvas = document.getElementById("draw");
    var ctx = canvas.getContext("2d");

    /**
     *  Lets resize our canvas to match our image.
     */
    canvas.width = numColumns;
    canvas.height = numLines;

    var width = canvas.width;
    var height = canvas.height;
    var iData = ctx.createImageData(numColumns, numLines);
    var d = iData.data;

    /**
     *  Calculate the signal mean and find the Sync A to start the drawing.
     */
    var dataMean = mean(fData, fData.length);
    var lineStartIndex = getSync(0, 22050, dataMean, fData).index;
    console.log("Start Line Index: "+lineStartIndex);

    /**
     *  This is the constants for the Correlation Algorithm.
     */
    var coeffT = 6.0;
    var searchSize = 40;
    var barOffset = 20;

    for(var line=0;line<numLines;line++) {
      // Slice just the piece of samples we need
      var lineData = fData.slice(lineStartIndex, lineStartIndex+samplesPerLine);
      // Resample it for our baudrate.
      lineData = resample(lineData, 4160, 11025, lowPass);

      // Fill the "color" data
      for (var column=0;column<numColumns;column++) {
        var value = lineData[column] * 256;
        d[(line*numColumns+column)*4]   = value;
        d[(line*numColumns+column)*4+1] = value;
        d[(line*numColumns+column)*4+2] = value;
        d[(line*numColumns+column)*4+3] = 255;
      }

      /**
       *  Look for our next SyncA Data so we can align the next line.
       *  This is for correcting Doppler Effect Curve in Image.
       */
      var sync = getSync(Math.floor(lineStartIndex+samplesPerLine-barOffset), searchSize, dataMean, fData);
      if (sync.score > coeffT)      //  If our correlation is high enough, it means we got a sync signal.
        lineStartIndex = sync.index;
      else
        lineStartIndex += samplesPerLine; //  If we don't find it (in very noise situations for example),
                                          //  we just assume that next line is next line.
    }
    // Yey! Finished :D
    console.log("Putting Image");
    ctx.putImageData(iData, 0,0);
  }
};

xhr.send();
