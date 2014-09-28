// hardcoded coords for Puerta del Sol
getToLat = 40.416711; getToLon = -3.703387;

function toRad(x) {
   return x * Math.PI / 180;
}

function toDeg(rad) {
  return rad * 180 / Math.PI;
}
function getBearing(lat1,lng1,lat2,lng2) {
    var dLon = toRad(lng2-lng1);
    lat1 = toRad(lat1);
    lat2 = toRad(lat2);
    var y = Math.sin(dLon) * Math.cos(lat2);
    var x = Math.cos(lat1)*Math.sin(lat2) - Math.sin(lat1)*Math.cos(lat2)*Math.cos(dLon);
    var rad = Math.atan2(y, x);
    var brng = toDeg(rad);
    return (brng + 360) % 360;
}

function getDistance(lat1, lon1, lat2, lon2) {
  // Calculate distance
  var R = 6371; // km 
  var x1 = lat2-lat1;
  var dLat = toRad(x1);
  var x2 = lon2-lon1;
  var dLon = toRad(x2);
  var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) *
    Math.sin(dLon/2) * Math.sin(dLon/2);
  var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a)); 
  return R * c;
}
function logPosition(position) {
  lat1 = position.coords.latitude; lon1 = position.coords.longitude;
  lat2 = getToLat; lon2 = getToLon;
  bearing_ang = getBearing(lat1,lon1,lat2,lon2);
  distance = getDistance(lat1,lon1,lat2,lon2);
  Pebble.sendAppMessage({bearing: bearing_ang*1000, distance: distance*1000, accuracy: position.coords.accuracy })
}

Pebble.addEventListener("ready",
  function(e) {
    navigator.geolocation.watchPosition(logPosition, null, {enableHighAccuracy: true});
  }
);
