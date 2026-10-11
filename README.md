# WaterFlow

Application de gestion WaterFlow (Qt 6 / C++17) : une seule fenêtre, une seule connexion,
une barre latérale commune et un module par rubrique.

| Rubrique         | État                                   | Dossier               |
|------------------|----------------------------------------|-----------------------|
| Personnel        | Intégré                                | `modules/personnel/`  |
| Livraisons       | Intégré (interface, données d'exemple) | `modules/livraisons/` |
| Autres rubriques | Page « bientôt disponible »            | —                     |

## Structure

```
src/                 main.cpp, MainWindow (sidebar + QStackedWidget), LoginDialog, style.h
modules/personnel/   personnel.{h,cpp,ui}, barchart.{h,cpp}
modules/livraisons/  livraisons.{h,cpp,ui}
```

Pour ajouter un module : créer un `QWidget` dans `modules/<nom>/`, l'ajouter au `CMakeLists.txt`
(et au `.pro`), puis remplacer la page « placeholder » correspondante dans `MainWindow::MainWindow`.

## Compilation

Prérequis : Qt 6 (Widgets) et CMake ≥ 3.16.

```bash
cmake -S . -B build
cmake --build build
./build/WaterFlow
```

Ou ouvrir `CMakeLists.txt` (ou `WaterFlow.pro`) dans Qt Creator.

## Connexion

Au premier lancement, un compte `admin` est créé avec un mot de passe par défaut défini dans
`src/logindialog.h`. Pensez à le changer avant tout usage réel.

## Données

Les employés sont enregistrés dans `employes.json` (dossier AppData de l'application).
Le module Livraisons affiche pour l'instant des données d'exemple.
