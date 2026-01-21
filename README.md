# ATC Simulation - Los Santos

Le but est de simuler des avions autonomes qui naviguent entre trois aéroports sur la carte de GTA V (LSIA, Sandy Shores, Grapeseed) en gérant leur carburant et les conflits d'accès aux pistes.

## Fonctionnement

Le système repose sur une architecture multi-threadée. Chaque avion est un thread indépendant qui calcule sa physique et gère son état, tandis qu'un thread central (le CCR) gère l'anti-collision et le routing global.

### Cycle de vie d'un vol

Les avions suivent une machine à états finis :

1. **Croisière** : Vol vers la destination (gestion basique des collisions en l'air).
2. **Approche** : Prise en charge par le contrôleur local, alignement.
3. **Atterrissage** : Si la tour donne l'autorisation (mutex sur la piste).
4. **Parking** : Roulage, arrêt moteur, et remplissage du réservoir (timer d'attente).
5. **Décollage** : Demande d'accès piste, décollage et choix d'une nouvelle destination aléatoire.

### Code couleur (Visuel)

L'état des avions est visible directement via leur couleur sur la map :

* **Cyan** : En croisière.
* **Magenta** : Atterrissage en cours.
* **Noir** : Au parking (moteurs coupés / on remet de l'essence).
* **Blanc** : Décollage.
* **Orange** : Fuel critique (< 20%).
* **Jaune** : Autres états (roulage, approche, etc.).

## Aspects Techniques

* **Multi-threading** : Utilisation de `std::thread` pour chaque avion + 1 thread pour le CCR.
* **Synchronisation** : `std::mutex` et `std::atomic` pour protéger les données partagées (position, accès piste, logs).
* **Logs** : Les événements (décollage, atterrissage, contact radar) sont écrits dans la console et dans un fichier `logs.json`.

## Aéroports gérés

* **LSIA** (Los Santos International)
* **Sandy Shores**
* **Lago Zancudo** (Grapeseed)