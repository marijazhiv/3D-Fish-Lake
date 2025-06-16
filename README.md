# 3D-Fish-Lake

Projekat prikazuje 3D scenu jezera sa ribicama koje plivaju kružnim putanjama. Jezero je plavo i okruženo peskom. Ribice su različitih veličina i boja, a brzina plivanja i prečnik putanja se kontrolišu tastaturom (W/S i A/D). Kamera je fiksna, iz ptičje perspektive, sa perspektivnom projekcijom i podrškom za zumiranje scroll točkićem.

## Karakteristike

- Prikaz imena, prezimena i indeksa autora na ekranu
- Ograničenje na 60 FPS
- Face culling i depth testing uključeni
- Izlazak iz aplikacije pritiskom na ESC
- Implementirano korišćenjem modernog OpenGL-a (bez fiksnog pipeline-a)

## Tehnologije

- C++
- OpenGL (moderni pristup)
- GLFW, GLEW, GLM
- stb_image za teksture
