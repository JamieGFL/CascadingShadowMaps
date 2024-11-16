# CascadingShadowMaps

Dieses Repo enthält all die Ressourcen des Bachelor-Seminar: Hardwarenahe 3D-Grafikprogrammierung im Wintersemester 2023/2024


## Beschreibung
Dieses Projekt wurde benutzt um mithilfe von LearnOpenGL OpenGL zu lernen als auch um den Code für das Seminar Hardwarenahe 3D-Grafikprogrammierung anzufertigen. Das Projekt liegt in cascadingshadowmaps und wurde in Clion angefertigt.  
Die main files für die Abgabe liegen in src/presentation, res/shader/presentation und res/texture/presentation. Weiter werden alle in /lib eingebundenen libraries genutzt.

## Usage
Es gibt zwei Executables, shadow_mapping und cascadedShadowmaps.

**shadow_mapping** ist eine Beispielanwendung für herkömmliches Shadow Mapping und sie auszuführen öffnet eine OpenGL Szene, in welcher die Szene durch verschiedene Inputs verändert werden kann.

**cascadedShadowmaps** ist eine Beispielanwendung für die Technik der Cascaded Shadow Maps und sie auszuführen öffnet eine OpenGL Szene, in welcher die Szene ebenfalls durch Inputs leicht verändert werden kann.  
Wichtig bei dieser Szene zu wissen ist, dass innerhalb des Hauses noch ein Point Light + Point Shadow implementiert sind, welche separat vom Direction Light und Cascaded Shadow Mapping sind.

Genauer Vorgang um das Programm auszuführen:

- Repo klonen mit git clone
- Submodules ziehen mit git submodule update --init --recursive
- Cmake Project loaden
- Projekt builden (Dauert aufgrund der Modelloading library Assimp eine Weile beim ersten Mal, aus Erfahrung war Visual Studio Compiler schneller als andere Compiler beim Building)
- (In IDE) Working Directory von beiden Executables auf cascaded_shadow_mapping ordner setzen (der, in welchem lib, res und src drin sind)
- Dann entweder cascadedShadowmaps oder shadow_mapping executable auszuführen, um die jeweilige Szene zu sehen.

Folgende Befehle können benutzt werden um die Szene zu verändern und sich in ihr rumzubewegen.

**shadow_mapping Inputs:**
- ESC - Fenster wird geschlossen
- WASD - es kann WASD benutzt werden um in der Szene rumzufliegen
- B - Basic Shadow Bias wird in Szene angewandt, Basic PCF wird ausgeschaltet, falls an
- P - Basic PCF wird in Szene angewandt, Basic Shadow Bias wird ausgeschaltet
- G - Peter Panning wird in Szene gezeigt durch zu hohen Shadow Bias
- O - Oversampling wird durch texture wrapping in Szene gefixed
- R - skybox can be changed to 3 available skyboxes

**cascadedShadowmaps Inputs:**
- ESC - Fenster wird geschlossen
- WASD - es kann WASD benutzt werden um in der Szene rumzufliegen
- Shift während WASD - Movement Speed erhöhen
- R - skybox can be changed to 3 available skyboxes
- Q - Kaskaden werden durch verschiedene Farben angezeigt
- 1 - 1 Kaskade wird benutzt
- 2 - 2 Kaskaden werden benutzt
- 3 - 5 Kaskaden werden benutzt


## Credits
**res/texture/presentation/natureModels:**  

"Lowpoly Nature Pack" (https://skfb.ly/otSsn) by Mongze is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).  
**Changes:** Models für die Szene wurden durch Blender ausgeschnitten und entsprechend gescaled, neue Texturen wurden applied


**res/texture/presentation/bench:**  
res/texture/presentation/bench
"Bench [LowPoly]" (https://skfb.ly/UEs8) by John Trent is licensed under Creative Commons Attribution-NonCommercial (http://creativecommons.org/licenses/by-nc/4.0/).
**Changes:** Models wurde in Blender für Szene entsprechend runtergescaled, neue Texturen wurden applied

**res/texture/presentation/house:**  
"Log Hut - Low Poly (V1)" (https://skfb.ly/oHrCx) by CircuitZ is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
**Changes:** Model wurde in Blender für Szene enstprechend runtergescaled, eine neue Textur wurde auch teilweise applied

**res/texture/presentation/table:**  
"Table" (https://skfb.ly/6XVnF) by Lasse Harm is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
**Changes:** Model wurde in Blender für Szene enstprechend runtergescaled

**res/texture/presentation/light:**  
"Simple Ceiling Light" (https://skfb.ly/6pMHq) by grimren13 is licensed under Creative Commons Attribution-NonCommercial (http://creativecommons.org/licenses/by-nc/4.0/).
**Changes:** Model wurde in Blender für Szene enstprechend runtergescaled, neeue Texturen wurden applied

**res/texture/presentation/skybox:** 
https://opengameart.org/content/retro-skyboxes-pack
is licensed under CC0 1.0 DEED https://creativecommons.org/publicdomain/zero/1.0/ (Public Domain)

