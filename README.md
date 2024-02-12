# StreetVendorsV-ScriptHookV [![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]
![Screenshot](https://github.com/SuleMareVientu/StreetVendorsV-ScriptHookV/blob/images/StreetVendorsV.png?raw=true)  
This mod aims to reintroduce the street vendors present in the previous GTA entry.  
Now you can walk up to a vendor stand and, if the vendor is present, buy a hotdog/burger for 5$ and heal your health, with fully working animations and a money system.  
Random peds can walk up to the vendors and eat, same as the player.  
Most of the behavior of the code was inspired by the decompiled GTA:IV scripts themselves, to remain as truthful to the original as possible ([vendor.sco](https://gist.github.com/SuleMareVientu/d94aad46046caa8d90fec093016d0414)).

**Requires [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/)**
## Installation: 
Simply extract the ASI inside the game's root directory (where you have installed GTA:V and Script Hook V, **not** inside the "scripts" folder).

## Changelog:
**v1.2** - Rewrote the entire script, it's now asynchronous. Random peds can now walk up to the vendors and eat.

**v1.1** - Added four rare models for vendors ("backup-vendors" inside ambientpedmodelsets.meta, only two points in the game).

## Credits:
- Inspired by [Working Hotdog Vendors](https://www.gta5-mods.com/scripts/working-hotdog-vendors) by [jedijosh920](https://www.gta5-mods.com/users/jedijosh920)
- alexguirre - [RAGE Parser Dumps](https://alexguirre.github.io/rage-parser-dumps/dump.html?game=gta5&build=3095#ePedConfigFlags)
- alloc8or - [Documentation](https://alloc8or.re/gta5/nativedb/)
- Alexander Blade - [ScriptHookV](http://www.dev-c.com/gtav/scripthookv/)

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg
