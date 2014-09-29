andevas
=======
![](screenshot.png)

andevas (spanish slang for "where'ya heading") is a location-aware Pebble app which will:
- Fetch your current position from your phone's GPS (using PebbleKit JS).
- Calculate the bearing and distance to that place (on the phone).
- Send that info to the Pebble using AppSync and then use Pebble's compass to show where the place is, with the distance (and precision) and an arrow pointing to it.

It is based on Pebble's own Compass example. This is why the UI is like a compass for now, but the north (dark half of the needle) is pointing to where the set place is.

Currently it has no settings screen, nor any way to change where it is pointing, which is currently hardcoded to [Puerta del Sol](https://www.google.com/maps/place/Puerta+del+Sol/@40.416947,-3.703528,17z/).

## TODO
- Make a proper UI (maybe a 3D arrow using also accelerometer data to match watch inclination)
- Be smarter about location-fetching (right now it just subscribes to HTML5 getLocation with high precision, which at least on Android uses GPS always). Maybe activating high precision only when distance < 500m or reducing lifecycles.
- Be able to configure the place where to go.
- Make it fetch places from [some sort of cloud geospatial database](http://cartodb.com/).
- Maybe, make it a watchapp instead of a watchface.
