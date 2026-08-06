# PC minigame asset map

This map records the first verified matches between the supplied `DataDump3`
sprite archive and the dedicated PC minigame sheets. Numeric names refer to the
original PNG names in the supplied archive. Black is the Clickteam transparent
colour key. The four-times enlarged sheets are reduced with nearest-neighbour
sampling before conversion to Wii U indexed RLE textures.

## BB's Air Adventure

| Purpose | DataDump3 PNG |
|---|---:|
| BB facing left | `716.png` |
| BB facing right | `717.png` |
| Small red balloon | `745.png` |
| Red balloon | `730.png` |
| Purple balloon | `776.png` |
| Green balloon | `777.png` |
| Orange balloon | `853.png` |
| Large red balloon | `854.png` |
| Exit cloud | `856.png` |

## Mangle's Quest

| Purpose | DataDump3 PNG |
|---|---:|
| Mangle left/right | `875.png`, `876.png` |
| Mangle parts and limbs | `882.png`–`890.png` |
| Child poses used by several minigames | `917.png`, `940.png` |

## Chica's Party

| Purpose | DataDump3 PNG |
|---|---:|
| Chica left/right | `907.png`, `908.png` |
| Cupcake variants | `913.png`, `915.png`, `923.png` |
| Crying child | `916.png` |
| Happy child | `917.png`, `940.png` |
| Room/platform tiles | `925.png`–`927.png` |

## Stage 01

| Purpose | DataDump3 PNG |
|---|---:|
| Stage 01 character poses | `471.png`, `472.png` |
| Crying child variants | `858.png`, `870.png` |
| Shared happy child | `917.png`, `940.png` |

## Happiest Day

| Purpose | DataDump3 PNG |
|---|---:|
| Puppet/child poses | `969.png`, `970.png` |
| Mask and child sprites | `971.png`, `974.png`, `975.png`, `982.png`–`995.png` |
| Balloons | `978.png`–`981.png` |

## Follow Me

The general archive contains the directional and dismantling frames around
`421.png`–`443.png`, plus the larger animatronic frames around `471.png` and
`472.png`. Exact object names and frame ownership remain to be confirmed
against the MFA event/frame data before they are wired into gameplay.

## Integration status

- The first Wii U texture pack contains BB, its balloon set and the exit cloud.
- The original generated BB drawing is replaced only at presentation level.
- The authentic room graph, platforms, collisions, jump physics and glitch
  route remain pending the MFA event audit.
- Other mini-games remain on their existing drawing layer until their object
  maps are confirmed.
