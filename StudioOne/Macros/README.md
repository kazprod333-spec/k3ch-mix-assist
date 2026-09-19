# Macro Organizer — rappel des FX Chains K3CH

Pas de script Host. Les commandes JS communautaires
(`Host.GUI.Commands.interpretCommand`) n’ont **pas** de couple
catégorie/action vérifié pour « appliquer FX Chain nommée au canal
sélectionné ». On reste sur l’UI Studio Pro 8.

## Procédure

1. Enregistre d’abord les FX Chains (`../FX-Chains/*/NOTES.txt`).
2. **Studio Pro 8 → Macro Organizer → New**.
3. Ajoute l’action d’insertion de **la FX Chain déjà stockée**
   (liste des FX Chains utilisateur dans le constructeur de macro —
   même principe que « stored FX chain → raccourci » décrit sur le
   forum utilisateurs pour les Event FX / inserts).
4. Nomme la macro comme la chaîne : `K3CH Voix Lead`, etc.
5. Assigne un raccourci clavier.
6. Dans la Console : sélectionne le canal cible → raccourci.

Suggestion de mapping (à adapter) :

| Raccourci | Macro / FX Chain |
|-----------|------------------|
| Ctrl+Alt+1 | K3CH Voix Lead |
| Ctrl+Alt+2 | K3CH Doubles |
| Ctrl+Alt+3 | K3CH Backs |
| Ctrl+Alt+4 | K3CH Bus Voix |
| Ctrl+Alt+5 | K3CH Drums |
| Ctrl+Alt+6 | K3CH 808 |
| Ctrl+Alt+7 | K3CH Melo |

K3CH Master (mode Inserts) affiche le même nom : copie le plan, construis,
puis la macro ne fait que *rappeler* ce que tu as déjà stocké.
