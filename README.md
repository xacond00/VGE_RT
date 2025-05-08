# Projekt VGE -- Akcelerační datové struktury pro ray-tracing 
## Bc. Ondřej Áč, xacond00; Bc. Jozef Bilko, xbilko03; Bc. Marek Konečný, xkonec86

### Instrukce ke kompilaci
V hlavním adresáři spustit tyto příkazy:
```
mkdir build
cd build
cmake ..
make
```
Ke spuštění jsou vyžadovány ".obj" soubory z tohoto repozitáře:
https://github.com/xacond00/VGE_RT

### Použité knihovny
SDL (github.com/libsdl-org/SDL)  
 - Slouží k tvorbě oken a zpracování vstupu (viz window.h, program.h)  
Imgui (github.com/ocornut/imgui )   
 - Slouží k tvorbě ovládacích prvků (viz. program.h)  
PTCR 2.0 (github.com/Panjaksli/PTCR2.0)  
 - Vlastní repozitář O. Áče, použity z něho byly funkce pro nalezení průsečíku trojúhelníku a AABB (poly.h, aabb.h).
 - Z matematických funkcí byla použita aproximace pro sinus/kosinus (fsin() v vec.h). 
 - Zbytek funkcionalit byl implementován úplně od základu, včetně vektorové knihovny, transformací, paprsků a ostatních náležitostí.
