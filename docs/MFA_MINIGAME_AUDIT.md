# MFA minigame audit

Source: supplied `fivenights3-94.mfa` (`MFU2`, approximately 119 MB).

This document records recoverable object and event labels from the original
Fusion project. It is intentionally conservative: labels are listed only when
they are present as readable project strings. Frame ownership is marked as
provisional until the frame/chunk structure is fully decoded.

## Follow Me object cluster

The project contains a dense Follow Me cluster with the following original
object labels:

- `shadow right`, `shadow up`, `shadow down`;
- `man 1`, `man run`, `take apart`;
- `f parts`, `bonnie parts`, `foxy parts`;
- `stage`, `chica`, `bonnie`, `chica walk`, `bonnie walk`, `foxy walk`;
- `arcade`, `trash1`, `party table`, `letters`;
- `hit box`, movement barriers and room-exit triggers;
- `freddy maker`, `spawn`, `puppet kill`;
- `controls`, `curtain`, `blocked`;
- `follow me`, `trigger`, `child`, `child 2`;
- `count trips`, `new frame`, `vertical`, `horizontal`.

This confirms that the MFA contains the original room-transition and
dismantling machinery rather than only image assets.

## Shared platform-minigame labels

Several minigame frames expose the same collision/sensor object pattern:

- `hitbox`;
- `feel right`, `feel left`, `feel top`;
- `feel bottom`, `feel bottom 2`, `feel bottom 3`;
- `watch screen`;
- `collect`, `collect 2`, `collect 3`, `collect 4`;
- `get this`;
- `secret`;
- `exit`;
- `cake`, `big cake`;
- `zip up`, `zip right`;
- `block`, `block 2`, `block 3`, `block 4`;
- `blip flash`;
- `extras game?`.

These labels will be used to reconstruct platform collision probes, secret
routes and completion conditions instead of approximating them from video.

## Named minigame/frame markers

Readable project markers include:

- `Toy Chica`;
- `stage01` (appears in two object clusters, likely normal and secret/path
  variants);
- `HAPPIEST DAY`;
- `Marion`;
- `balloons`;
- `long glitched2`;
- `jump2`;
- `land`;
- `good end`, `goodend`, `cutscene`, `load`.

## Immediate implementation order

1. Finish the PC sprite presentation for BB's Air Adventure.
2. Decode the first shared sensor/collision cluster into a portable room
   definition.
3. Replace BB's free movement with platform gravity, collision probes and room
   transitions from the MFA.
4. Reuse the same room engine for Mangle's Quest, Chica's Party, Stage 01 and
   Happiest Day.
5. Handle Follow Me separately because it contains animated walking and
   dismantling sequences.
