# PC Phantom and Springtrap attack sequences

The final Wii U gameplay renderer uses the original PC FNaF 3 General Sprites image bank for Springtrap and the Phantoms. The Phantom image ordering is taken from the object animation records in the supplied `fivenights3-94.mfa`, not from the older PSX port.

PC attack sequences used by gameplay:

- Phantom Foxy (`foxy`): 13 MFA images;
- Phantom Balloon Boy (`BB scare`): 11 MFA images;
- Phantom Chica (`chica 2`): 15 MFA images;
- Phantom Freddy (`fscare`): 21 MFA images;
- Springtrap: 45 continuous PC General Sprites frames.

Phantom Freddy's office pass also uses the two animations stored by the PC `fwalk` object: a 14-image walk cycle and an 11-image duck/crouch cycle.

The non-contiguous General Sprites IDs are intentional. Clickteam stores the images in one global bank, so unrelated UI/minigame images can sit numerically between consecutive frames of a Phantom animation. `tools/convert_pc_character_visuals.py` therefore keeps the MFA image lists verbatim and validates that every required PNG exists before generating Wii U RLE textures.

The PC Sound Effects archive supplies `scream3.wav`, `garble1.wav`, `mask.wav`, `echo1.wav`, `echo3b.wav` and `echo4b.wav`; these are converted directly to the Wii U PCM data used at runtime. The downloaded archive is SHA-256 pinned by `tools/prepare_generated_assets.sh`.

Phantom Mangle and Phantom Puppet keep their non-standard attacks rather than receiving invented jumpscare sequences. Mangle uses its exact PC CAM04 composite, then enters the garble/audio-failure state. Puppet uses its exact PC CAM08 composite and PC `phantom head` office art, blocks input and ultimately causes a ventilation failure.

The PSX source is still fetched by the build as a legacy/fallback asset source for older renderer code paths, but the final Springtrap/Phantom gameplay presentation is routed through the PC character and camera-fidelity layers.
