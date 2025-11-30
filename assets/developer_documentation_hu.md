# Fordítás menete
- jelenleg csak Windows rendszeren lehetséges Visual Studio használatával
1. Navigáljunk a projekt mappájába
2. Töltsük le a külső könyvtárakat: `git submodule update --init --recursive`
3. Generáljuk le a projektfájlokat: `premake5 vs2022`
4. Nyissuk meg Visual Studioban a generált `PlaneLab.sln` fájlt
5. Nyomjuk meg az F5-öt
6. Ezután a program sikeres lefordulást követően el kell induljon
# Felhasznált könyvtárak
- [GLFW](https://www.glfw.org/): ablakok létrehozására és azokra OpenGL-lel történő rajzolásra különböző operációs rendszereken
- [Dear ImGui](https://github.com/ocornut/imgui): felhasználói felület egyszerű létrehozásához (nem csak) OpenGL környezetben
	- az operációs rendszerek saját megoldásaihoz képest ez teljes mértékben cross platform
- [ImPlot](https://github.com/epezent/implot): a fenti ImGui-ra épülő könyvtár, amely segítségével adatgrafikonokat rajzolhatunk ki a felhasználói felületre
- [cimgui](https://github.com/cimgui/cimgui): mivel az ImGui C++-ban íródott, a C nyelvvel való használathoz ahhoz C-ben írt függvénydefiníciók szükségesek
- [cimplot](https://github.com/cimgui/cimplot): mivel az ImGui és az ImPlot C++-ban íródott, a C nyelvvel való használathoz ezekhez C-ben írt függvénydefiníciók szükségesek
- [nativefiledialog](https://github.com/mlabbe/nativefiledialog): fájlválasztó ablak létrehozására különböző operációs rendszereken
- [stb_image](https://github.com/nothings/stb): tömörített képek dekódolására

# A projekt felépítése
- felépítés:
	- `assets`: git repo képei
	- `build`: generált projektfájlok
	- `lib`: külső könyvtárak forráskódjai
	- `tools`: fordítást segítő scriptek
	- `resources`: a beágyazni kívánt fájlok
	- `src`: a program forráskódja
		- `resources`: automatikusan generált fájlokat tartalmaz, amelyek C forráskódként beágyazva tartalmazzák a `../resources` mappa elemeit
			- ezek generálása a projektfájlok legenerálásával egyszerre fut le (`premake5 vs2022`)
			- a `tools/embed_resource.lua` script végzi
		- `data_structures`: segéd adatstruktúrákat tartalmaz, amelyeket a program mindenhol használ
			- `linked_list.c/h`: láncolt lista
			- `dynamic_buffer.c/h`: dinamikus méretű tömb
			- `dynamic_string.c/h`: dinamikus méretű karakterlánc
		- `resource_management`: külső adatok betöltésére és kezelésére való segédfüggvények
			- `ico_file.c/h`: .ico fájlokat kezel, illetve alakít .bmp formátumba
			- `texture.c/h`: OpenGL textúrák létrehozása és felszabadítása
		- `shapes`:
			- `shape.c/h`: alakzat szülő osztály
			- `line/circle/hyperbola/parabola.c/h`: ábrázolható alakzatok
		- `window`: GLFW és ImGui osztályokba csomagolva
		- `graph_window.c/h`: egy projekt ablakának osztálya
		- `graph.c/h`: egy projekt által tárolt adatokat kezelő osztály
		- `winmain.c`: Windows rendszerre fordításhoz szükséges WinMain függvényt tartalmazza, ami csak meghívja a sima main-t egy debug konzol ablak létrehozása mellett
- a program objektum-orientáltságra alapul, amit C-ben kicsit trükkös megvalósítani
	- ablak osztály -> ablak imguival osztály -> fő ablak osztály
	- alakzat osztály -> egyenes, parabola, kör, stb. osztályok
	- láncolt lista osztály
	- textúra osztály (segéd OpenGL textúrák tárolására és betöltésére)
	- graph window (projekt ablak) példányokat tartalmaz a main window (fő ablak)
- további segédfüggvények és makrók találhatóak a `utils.h/c` fájlban
- a `main.c` fájl `main` függvénye kizárólagos szerepe hogy létrehozzon egy `MainWindow` objektumot és meghívja a `run_main_loop` metódusát
- a forráskódról részletesebb dokumentáció (függvények, adatstruktúrák listája) a doxygen mappában található