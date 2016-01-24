/*jslint node: true */
"use strict";

var Sequelize = require("sequelize");

/**
 *  Most data and info from http://spaceflight.nasa.gov/realdata/sightings/SSapplications/Post/JavaSSOP/SSOP_Help/tle_def.html
 */

var SatelliteDatabase = function(sequelize) {

  this.TLE = sequelize.define("TLE", {
    satellite_number        : { type: Sequelize.INTEGER, allowNull: false, primaryKey: true, comment: "Satellite Number" },
    name                    : { type: Sequelize.TEXT, comment: "Satellite Name" },
    classification          : { type: Sequelize.STRING(1), comment: "Satellite Classification" },
    international_designator: { type: Sequelize.STRING(8), comment: "International Designator. Ex. 84123A launched at year 84 and was the 124th launch of the year. \"A\" means it was the first object from the launch." },
    epoch_year              : { type: Sequelize.INTEGER, comment: "Epoch Year" },
    epoch_day               : { type: Sequelize.DECIMAL(12, 8), comment: "Epoch Day. Julian Day Fraction couting from Jan 1st of the Epoch Year." },
    ballistic_coefficient   : { type: Sequelize.DOUBLE, comment: "Balistic Coefficient. It is the daily rate of change in number of revolutions that objects complete by day divided by two. Units are Revolutions / Day" },
    second_derivative_motion: { type: Sequelize.DOUBLE, comment: "Second Derivative of Mean Motion. Second order drag term used for calculating orbit decay." },
    drag_term               : { type: Sequelize.DOUBLE, comment: "Drag Term or Radiation Pressure Coefficient (BSTAR)" },
    ephemeris_type          : { type: Sequelize.INTEGER, comment: "Ephemeris Type. Currently not used and will always be 0" },
    element_set_number      : { type: Sequelize.INTEGER, comment: "Element Set Number. Incremented when a new TLE is generated for the object." },
    inclination             : { type: Sequelize.DECIMAL(7, 4), comment: "Inclination (degrees). The angle between the equator and the orbit plane" },
    right_ascension         : { type: Sequelize.DECIMAL(7, 4), comment: "Right Ascension of the Ascending Node (degrees). The angle between vernal equinox and the point where the orbit crosses the equatorial plane (going north)." },
    eccentricity            : { type: Sequelize.DECIMAL(7, 7), comment: "Eccentricity. A constant defining the shape of the orbit (0=circular, Less than 1=elliptical)" },
    perigee_argument        : { type: Sequelize.DECIMAL(7, 4), comment: "Argument of Perigee. The angle between the ascending node and the orbit's point of closest approach to the earth (perigee)" },
    mean_anomaly            : { type: Sequelize.DECIMAL(7, 4), comment: "Mean Anomaly (degrees). The angle, measured from perigee, of the satellite location in the orbit referenced to a circular orbit with radius equal to the semi-major axis." },
    mean_motion             : { type: Sequelize.DECIMAL(10, 8), comment: "Mean Motion. The value is the mean number of orbits per day the object completes" },
    revolution_number       : { type: Sequelize.INTEGER, comment: " The orbit number at Epoch Time. This time is chosen very near the time of true ascending node passage as a matter of routine" },
    tle_data                : { type: Sequelize.TEXT, comment: "RAW TLE Data" }
  }, {
    underscored: true,
    indexes: [
      {
        name: 'tle_satellite_number',
        method: 'BTREE',
        fields: ['satellite_number']
      }
    ],
    classMethods: {
      fromTLE: function(tledata, tle) {
        tledata = tledata.split("\n");

        if (tledata.length != 3)
          return {data: null, err: "TLE Data not in Three Line format!"};


        var name                      = tledata[0].slice(2,tledata[0].length).trim();
        var satNum                    = tledata[1].slice(2,7).trim();
        var elementSetNumber          = parseInt(tledata[1].slice(64,68).trim());
        var classification            = tledata[1][7];
        var international_designator  = tledata[1].slice(9,17).trim();
        var epoch_year                = parseInt(tledata[1].slice(18,20).trim());
        var epoch_day                 = parseFloat(tledata[1].slice(20,32).trim());
        var ballistic_coefficient     = parseFloat(tledata[1].slice(33,43).trim());
        var second_derivative_motion  = parseFloat(tledata[1].slice(44,50).trim()) * Math.pow(10,parseFloat(tledata[1].slice(50,52).trim()));
        var drag_term                 = parseFloat(tledata[1].slice(53,59).trim()) * Math.pow(10,parseFloat(tledata[1].slice(59,61).trim()));
        var ephemeris_type            = parseInt(tledata[1][62]);
        var element_set_number        = parseInt(tledata[1].slice(64,68).trim());
        var inclination               = parseFloat(tledata[2].slice(8,16).trim());
        var right_ascension           = parseFloat(tledata[2].slice(17,25).trim());
        var eccentricity              = parseFloat("0." + tledata[2].slice(26,33).trim());
        var perigee_argument          = parseFloat(tledata[2].slice(34,42).trim());
        var mean_anomaly              = parseFloat(tledata[2].slice(43,51).trim());
        var mean_motion               = parseFloat(tledata[2].slice(52,63).trim());
        var revolution_number         = parseInt(tledata[2].slice(63,68).trim());

        // TODO: Checksum check

        if (tle) {
          if (tle.element_set_number >= element_set_number)
            return {data: tle, err: "TLE Data is already up to date."};

          tle.element_set_number = elementSetNumber;
          tle.classification = classification;
          tle.international_designator = international_designator;
          tle.epoch_year = epoch_year;
          tle.epoch_day = epoch_day;
          tle.ballistic_coefficient = ballistic_coefficient;
          tle.second_derivative_motion = second_derivative_motion;
          tle.drag_term = drag_term;
          tle.ephemeris_type = ephemeris_type;
          tle.inclination = inclination;
          tle.right_ascension = right_ascension;
          tle.eccentricity = eccentricity;
          tle.perigee_argument = perigee_argument;
          tle.mean_anomaly = mean_anomaly;
          tle.mean_motion = mean_motion;
          tle.revolution_number = revolution_number;
          tle.tle_data = tledata.join("\n");
        } else {
          tle = this.build({
            name: name,
            satellite_number: satNum,
            element_set_number: elementSetNumber,
            classification: classification,
            international_designator: international_designator,
            epoch_year: epoch_year,
            epoch_day: epoch_day,
            ballistic_coefficient: ballistic_coefficient,
            second_derivative_motion: second_derivative_motion,
            drag_term: drag_term,
            ephemeris_type: ephemeris_type,
            inclination: inclination,
            right_ascension: right_ascension,
            eccentricity: eccentricity,
            perigee_argument: perigee_argument,
            mean_anomaly: mean_anomaly,
            mean_motion: mean_motion,
            revolution_number: revolution_number,
            tle_data: tledata.join("\n")
          });
          return { data: tle, err: null };
        }
      }
    },
    instanceMethods: {
      toTLE: function() {
        return this.tle_data;
      }
    }
  });

  this.TrackSat = sequelize.define("TrackSat", {
    track_id        : { type: Sequelize.INTEGER, primaryKey: true, autoIncrement: true, comment: "Tracking ID" },
    type            : { type: Sequelize.ENUM('weather', 'transponder', 'other'), defaultValue: 'other', comment: "Tracking Type" },
    enabled         : { type: Sequelize.BOOLEAN, defaultValue: true, comment: "If the tracking is enabled" },
    satellite_number: { type: Sequelize.INTEGER, unique: true, references: {
        model: this.TLE,
        key: 'satellite_number',
        deferrable: Sequelize.Deferrable.INITIALLY_IMMEDIATE
      }
    }
  },{
    underscored: true
  });

  this.TrackSat.belongsTo(this.TLE, { foreignKey: 'satellite_number' });

  this.Transponder = sequelize.define("Transponder", {
    transponder_id  : { type: Sequelize.INTEGER, primaryKey: true, autoIncrement: true, comment: "Transponder ID" },
    name            : { type: Sequelize.TEXT, comment: "Transponder Name" },
    mod_type        : { type: Sequelize.ENUM('AFSK', 'FM', 'APT', 'LRPT', 'HRPT', 'Other'), defaultValue: 'Other', comment: "Modulation Type" },
    mod_subtype     : { type: Sequelize.ENUM('Packet', 'Voice', 'APRS', 'SSTV', 'Other'), defaultValue: 'Other', comment: "Modulation Sub Type" },
    mod_bitrate     : { type: Sequelize.INTEGER, defaultValue: 0, comment: "Modulation Bitrate (for digital modes)"},
    uplink          : { type: Sequelize.DECIMAL(12, 5), defaultValue: 0.0, comment: "Uplink Frequency in MHz"},
    downlink        : { type: Sequelize.DECIMAL(12,5), defaultValue: 0.0, comment: "Downlink Frequency in MHz"},
    description     : { type: Sequelize.TEXT, comment: "Transponder Description" },
    satellite_number: { type: Sequelize.INTEGER, references: {
        model: this.TLE,
        key: 'satellite_number',
        deferrable: Sequelize.Deferrable.INITIALLY_IMMEDIATE
      }
    }
  },{
    underscored: true
  });
};

SatelliteDatabase.prototype.initialize = function() {
  var _this = this;
  console.log("Creating TLE Table");
  return this.TLE.sync().then(function() {
    console.log("Creating Satellite Tracking Database");
    return _this.TrackSat.sync();
  }).then(function() {
    console.log("Creating Transponder Database");
    return _this.Transponder.sync();
  });
};

SatelliteDatabase.prototype.addOrUpdateTrackSat = function(tracksatdata, cb) {
  var _this = this;
  this.TrackSat.findOne({
    where: {
      satellite_number: tracksatdata.satellite_number
    }
  }).then(function (tracksat) {
    if (tracksat === null) {
      tracksat = _this.TrackSat.build(tracksatdata);
    } else {
      tracksat.enabled = tracksatdata.enabled !== null ? tracksatdata.enabled : tracksat.enabled;
      tracksat.type = tracksatdata.type !== null ? tracksatdata.type : tracksat.type;
    }
    tracksat.save().then(function() {
      cb(null, tracksat)
    }).catch(function(error) {
      cb(error, null);
    });
  })
};

SatelliteDatabase.prototype.addOrUpdateTransponder = function(transponder_data, cb) {
  var _this = this;
  if (transponder_data.transponder_id !== null) {
    this.Transponder.findOne({
      where: {
        transponder_id: transponder_data.transponder_id
      }
    }).then(function(transponder) {
      if (transponder === null) {
        transponder = _this.Transponder.build(transponder_data);
      } else {
        transponder.name = transponder_data.name !== null ? transponder_data.name : transponder.name;
        transponder.mod_type = transponder_data.mod_type !== null ? transponder_data.mod_type : transponder.mod_type;
        transponder.mod_subtype = transponder_data.mod_subtype !== null ? transponder_data.mod_subtype : transponder.mod_subtype;
        transponder.mod_bitrate = transponder_data.mod_bitrate !== null ? transponder_data.mod_bitrate : transponder.mod_bitrate;
        transponder.uplink = transponder_data.uplink !== null ? transponder_data.uplink : transponder.uplink;
        transponder.downlink = transponder_data.downlink !== null ? transponder_data.downlink : transponder.downlink;
        transponder.description = transponder_data.description !== null ? transponder_data.description : transponder.description;
      }
      transponder.save().then(function() {
        cb(null, transponder)
      }).catch(function(error) {
        cb(error, null);
      });
    });
  } else {
    var transponder = this.Transponder.build(transponder_data);
    transponder.save().then(function() {
      return cb(null, transponder);
    }).catch(function(error) {
      cb(error, null);
    });
  }
};

SatelliteDatabase.prototype.getTrackSats = function(cb) {
  this.TrackSat.findAll({
    include: [{model: this.TLE}]
  }).then(function(data) {
    return cb(null, data);
  }).catch(function(error) {
    return cb(error, null);
  });
};

SatelliteDatabase.prototype.getTransponders = function(satnumber, cb) {
  this.Transponder.findAll({
    where: {
      "satellite_number": satnumber
    }
  }).then(function(data) {
    return cb(null, data);
  }).catch(function(error) {
    return cb(error, null);
  });
};

SatelliteDatabase.prototype.addOrUpdateTLE = function(tledata, cb) {
  var _this = this;
  var tledata2 = tledata.split("\n");
  if (tledata2.length !== 3)
    return cb("TLE Data not in Three Line format!");

  var satNum                    = tledata2[1].slice(2,7).trim();
  var elementSetNumber          = parseInt(tledata2[1].slice(64,68).trim());

  this.TLE.findOne({
    where: {
      satellite_number: satNum
    }
  }).then(function(tle) {
    var proc;
    if (tle !== null) {
      if (tle.element_set_number >= elementSetNumber)
        return cb("Database TLE is more recent.", tle);
      proc = _this.TLE.fromTLE(tledata, tle);

      if (proc.err)
        return cb(proc.err, proc.data);

      proc.data.save().then(function() {
        cb(null, proc.data);
      }).catch(function(error) {
        cb(error, null);
      });
    } else {
      proc = _this.TLE.fromTLE(tledata);
      if (proc.err)
        return cb(proc.err, proc.data);

      proc.data.save().then(function() {
        cb(null, proc.data);
      }).catch(function(error) {
        cb(error, null);
      });
    }
  });
};

module.exports = SatelliteDatabase;