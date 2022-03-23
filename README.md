# Map Generator
Project to create a random fantasy map. My motivation for the project came from a game I was trying to make a while back, which required a randomly generated map, so I decided to make one myself. I tried to make one using voronoi diagrams at that time, but got stuck in it and left both the game and the map. The idea never left me though, and I started work on the map generation again, even though the game it was supposed to be used for, has been abandoned.
### Screenshots
<img src="https://github.com/notSanil/Map-Generation/blob/master/images/map1.png?raw=true" width="300"><img src="https://github.com/notSanil/Map-Generation/blob/master/images/map2.png?raw=true" width="300">

### The Approach
I started out with generating voronoi diagrams, they are used because they produce a really organic looking landscape without having to alter them too much. The approach I used has been popularised by Amit P. in his blog. Basically, it involves creating a bunch of random points, creating a voronoi diagram out of them, and then relaxing those points a few times (4 in my case) to smooth them out.

Next step was to figure out a way to generate a heightmap, and assign it to the polygons created by voronoi triangulation. Perlin noise comes to the rescue here. One of the great things about Perlin Noise is that it generates a smooth heightmap, and you can layer a bunch of noise together to create as much detail as you want. It does have a drawback however, it is normally distributed. This means that a lot more numbers will be generated close to it's middle (0.5 in this case) rather than its edges. Therefore we need to do a few mathematical operations on it in order to make it usable and generate a more realistic terrain. 

### Scope for improvement
* The terrain generated does not have a lot of detail, which can be improved by increasing the noisiness in the map.
* There aren't nearly enough biomes to properly depict the change in height.
* The biomes just sort of abruptly start and end, there is no blending between them.
* The biomes are assigned purely on the basis of heights, a temperature map overlayed on top of the heightmap would lead to a much more realistic result.
* There are no water features other than lakes and oceans in the map, having some rivers could definitly liven up the maps.

### References
[Martin O' Leary's map generator](http://mewo2.com/notes/terrain/) which explores a different and a more realistic approach for creating a map by simulating natural processes.

[Amit P's map gen notes](http://www-cs-students.stanford.edu/~amitp/game-programming/polygon-map-generation/), which I have used for voronoi diagrams, graph representaion, and Perlin Noise manipulation.

[Asgaar's map generator](https://azgaar.github.io/Fantasy-Map-Generator/), which is hands down the best I've seen till date.
