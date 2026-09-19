# Studio Pro 8 — pack compagnon K3CH Master

**Maison K3CH Production — Alger**

Ce dossier n’est **pas** un plug-in et ne s’installe pas dans le Master VST3.
Il documente le flux **Studio Pro 8** (Fender) pour rappeler des
chaînes d’inserts sur d’autres canaux Console.

## Limite honnête

Un VST3 (y compris **K3CH Master** et **K3CH Presets**) **ne peut pas** :

- charger un plugin sur un autre canal ;
- appliquer une FX Chain à distance via une API hôte publique vérifiée ;
- piloter la Console comme un script mixer.

L’application d’une chaîne sur un autre track / canal se fait **dans Studio Pro 8** :
FX Chains (Navigateur) ou **Macro Organizer**.

Aucun script `.package` n’est fourni : les références communautaires exposent
`Host.GUI.Commands.interpretCommand(categorie, action)` pour des commandes
d’édition (Select All, etc.), mais **aucune paire catégorie/action documentée
et vérifiée** ne charge une FX Chain nommée sur le canal sélectionné.
On ne l’invente pas. Utilise les macros UI.

## Où poser les produits

| Plug-in | Où | Rôle |
|---------|----|------|
| K3CH Master | Canal **Master** | Mode Inserts = plans (thru). Mode Master = DSP mastering. |
| K3CH Presets | Canal voix / piste choisie | Chaîne live *dans* le plug-in. |
| K3CH Plugin | N’importe quel insert | Encyclopédie, pass-through. |

## Enregistrer une FX Chain (une fois)

1. Sélectionne un canal Console (ex. voix lead).
2. Pose les inserts dans l’ordre du plan (copie depuis K3CH Master → **Copier le plan**).
   Pour la voix, commence souvent par **K3CH Presets** sur *ce* canal.
3. Flèche des inserts → **Store FX Chain** (ou Channel Editor → store).
4. Nomme-la **exactement** comme dans le plan, ex. `K3CH Voix Lead`.
5. Dossier suggéré : sous-dossier **K3CH** dans tes FX Chains.

### Dossiers User Data (chemins disque)

Les FX Chains vivent sous l’emplacement **User Data**
(Studio Pro → Options / Préférences → Locations), pas dans ce dépôt.
Il n’y a qu’un seul User Data actif.

Studio Pro 8 (Fender) enregistre les **nouvelles** FX Chains ici :

- Windows : `Documents\StudioPro_UserData\Presets\Fender\FX Chains\`
- macOS : `~/Documents/StudioPro_UserData/Presets/Fender/FX Chains/`

Si tu as gardé l’ancien root User Data après upgrade : `…/Presets/Fender/FX Chains/`
(le dossier racine peut encore s’appeler `Studio One` sur le disque).

Versions PreSonus **Studio One 4–7** (legacy, encore lues) :

- Windows : `Documents\Studio One\Presets\PreSonus\FX Chains\`
- macOS : `~/Documents/Studio One/Presets/PreSonus/FX Chains/`

L’hôte peut afficher un mélange PreSonus + Fender. Pour du neuf : stocke sous **Fender**, puis Navigateur → Home → **Re-Index Presets**.

Les dossiers `StudioOne/FX-Chains/K3CH_*` de ce repo sont des **stubs**
(noms + NOTES). Copie les NOTES, construis la chaîne dans Studio Pro 8, *Store*.
L’hôte n’importe pas ces dossiers vides comme presets.

## Rappeler une FX Chain

- **Navigateur → Effects → FX Chains** : glisse `K3CH Voix Lead` sur le canal.
- Ou inserts du canal → dossier FX Chains → choisir le preset.

## Macro Organizer (raccourci)

1. **Studio Pro 8 → Macro Organizer** (ou barre macros).
2. **New**.
3. Ajoute la commande qui **insère la FX Chain déjà stockée**
   (dans l’UI macros : choisis la FX Chain nommée, comme pour un Event FX /
   insert chain — flux documenté côté utilisateurs).
4. Assigne un raccourci (ex. Ctrl+Alt+1 = Voix Lead, Ctrl+Alt+2 = Doubles…).
5. Sélectionne le canal cible dans la Console, déclenche la macro.

Les noms alignés sur le VST :

| Preset Master (mode Inserts) | Nom FX Chain |
|------------------------------|--------------|
| Voix lead | `K3CH Voix Lead` |
| Doubles | `K3CH Doubles` |
| Backs / BGV | `K3CH Backs` |
| Bus voix | `K3CH Bus Voix` |
| Drums | `K3CH Drums` |
| 808 | `K3CH 808` |
| Mélo | `K3CH Melo` |

Détail des inserts : `FX-Chains/*/NOTES.txt` et l’onglet Inserts du plug-in.

## Master en fin de mix

1. K3CH Master est déjà sur le **Master**.
2. Passe **Inserts / Mix → Master**.
3. Choisis un preset (Streaming -14 LUFS, Club / Loud, …) → **Appliquer**.
4. Ajuste EQ / sat / largeur / limiteur. Mesure LUFS + true peak dans Studio Pro 8
   (le plug-in vise un plafond, ce n’est pas un loudness meter intégré).
