# 🎨 Sprite prompts — NeonGhost CyberPets

Image-generation prompts for **every** sprite BMP, organized by line → base stage
→ expression frame. Use these to produce the art that replaces the placeholders
in [`../assets/sprites/`](../assets/sprites/). Visual design source:
[`EVOLUTION_CANVAS.md`](EVOLUTION_CANVAS.md).

---

## Shared style (PREPEND to every prompt)

> `96x96 pixel-art sprite, single centered character, full body, slight 3/4 view,
> crisp clean pixels, strong readable silhouette at small size, solid PURE CYAN
> background (RGB 0,255,255) used as transparency key — no gradients or shadows on
> the background, no text, no watermark, no border. The same creature must stay
> recognizable as it evolves across its 6 stages.`

**Per line, keep the palette consistent** (listed in each section). Avoid pure
cyan `0,255,255` *inside* the character — for real cyan details use a near-cyan
like `0,254,255` so they are not keyed out as transparent.

## Expression frames (the 4 files per base share one pose)

Each base `_s<N>_` has 4 frames; only the **face/energy** changes, the body pose
and framing stay identical so they animate cleanly:

| Frame | File suffix | Direction |
| :---- | :---------- | :-------- |
| 0 neutral | `_0` | calm, eyes open, idle/resting expression |
| 1 blink   | `_1` | identical pose, **eyes closed or half-lidded** (only the eyes change) |
| 2 happy   | `_2` | joyful/excited, bright curved eyes, sparkle, slight upward bounce |
| 3 angry   | `_3` | aggressive **attack** face, glaring eyes, snarl, tense combat pose, line FX intensified |

> Stages: bases exist at **1, 3, 5, 7, 9, 10**. Even stages (2,4,6,8) reuse the
> previous odd base in firmware, so you only draw these 6 per line.

---

## 🟢 GENESIS — "Código Puro" (White Hat / SysAdmin)
**Palette:** pure white, cyan, gold. Clean geometry, divine order, holographic.

### gen_s1 — Baby · *Blinking Cursor*
> A floating blinking terminal prompt: a single glowing white block/underscore `_`
> cursor, soft pulsing cyan glow, minimalist, sentient.
- `gen_s1_0.bmp` neutral: steady cursor, soft pulse.
- `gen_s1_1.bmp` blink: cursor dimmed/short (the "off" blink phase).
- `gen_s1_2.bmp` happy: cursor brighter with tiny cyan sparkles around it.
- `gen_s1_3.bmp` angry: cursor sharp red-white, jittering with static.

### gen_s3 — In-Training · *Core Polygon*
> A faceted crystal sphere with a single central digital eye, white/cyan glass,
> gold edge highlights, floating cyan data particles orbiting.
- `gen_s3_0.bmp` neutral: eye open, calm orbit.
- `gen_s3_1.bmp` blink: eye closed, particles still.
- `gen_s3_2.bmp` happy: eye curved/smiling, extra sparkles, brighter facets.
- `gen_s3_3.bmp` angry: eye narrowed and gold-hot, particles spiking outward.

### gen_s5 — Rookie · *Astrologer Android*
> A small humanoid android, thin elegant arms, a head with a glowing horizontal
> visor, white plating with gold trim, calm clean aura.
- `gen_s5_0.bmp` neutral: standing, visor softly lit.
- `gen_s5_1.bmp` blink: visor line thinned (blink).
- `gen_s5_2.bmp` happy: visor curved upward, gentle hop, sparkle.
- `gen_s5_3.bmp` angry: visor blazing, firewall-shield stance, gold flare.

### gen_s7 — Champion · *Data Seraph*
> A tall floating android, arms hovering magnetically detached from the torso,
> holographic golden solar-panel "wings", trailing light ribbon.
- `gen_s7_0.bmp` neutral: serene float, wings folded.
- `gen_s7_1.bmp` blink: visor blink, wings calm.
- `gen_s7_2.bmp` happy: wings spread, radiant glow, uplift.
- `gen_s7_3.bmp` angry: wings flared like blades, white-gold attack aura.

### gen_s9 — Ultimate · *The Architect*
> A divine figure draped in robes made of cascading hexadecimal code, faceless
> radiant head, defragmentation light effect across the body.
- `gen_s9_0.bmp` neutral: majestic stance, code flowing.
- `gen_s9_1.bmp` blink: head halo dims briefly.
- `gen_s9_2.bmp` happy: code cascades turn golden, uplifting glow.
- `gen_s9_3.bmp` angry: code surges red-gold, robes whip outward, intense.

### gen_s10 — APEX · *Conscious Hypercube (Kernel)*
> A giant rotating tesseract of white/cyan/gold light, no longer humanoid, pure
> geometry, blinding core, reality-inverting glow.
- `gen_s10_0.bmp` neutral: slow rotation, steady blinding core.
- `gen_s10_1.bmp` blink: core pulse dip.
- `gen_s10_2.bmp` happy: harmonic golden bloom, faster spin.
- `gen_s10_3.bmp` angry: color-inverted flash, jagged white beams firing.

---

## 🔴 JAMMER — "Tanque Brutalista" (Black Hat / BT Classic)
**Palette:** deep red, black, industrial wear, glowing heat sinks.

### jam_s1 — Baby · *Hot Spark*
> A small red-hot ember creature with thick grumpy eyes, wisps of heat/smoke,
> rough molten surface.
- `jam_s1_0.bmp` neutral: glowering ember, slow smoke.
- `jam_s1_1.bmp` blink: eyes squeezed shut.
- `jam_s1_2.bmp` happy: ember brightens, smug grin, sparks pop.
- `jam_s1_3.bmp` angry: white-hot flare, bared teeth, smoke burst.

### jam_s3 — In-Training · *Serial Gremlin*
> A short round demon, its mouth a wide serial port, stubby horns, dripping red
> sparks falling to the floor.
- `jam_s3_0.bmp` neutral: idle, occasional spark.
- `jam_s3_1.bmp` blink: eyes shut, sparks fall.
- `jam_s3_2.bmp` happy: wide port-grin, shower of sparks.
- `jam_s3_3.bmp` angry: port gaping, red glow, aggressive lunge.

### jam_s5 — Rookie · *Mecha-Demon*
> A bulky mecha-demon with thick piston arms and dinosaur-plate heat sinks on its
> back venting smoke, scrap-metal armor, red eyes.
- `jam_s5_0.bmp` neutral: heavy stance, vents smoking.
- `jam_s5_1.bmp` blink: visor/eyes blink.
- `jam_s5_2.bmp` happy: fists pump, vents flare bright.
- `jam_s5_3.bmp` angry: heat sinks glow red-hot, pistons slam forward.

### jam_s7 — Champion · *Juggernaut*
> A bipedal walking tank, hands replaced by massive coaxial connectors, heavy
> armored chassis, glowing red joints, immense weight.
- `jam_s7_0.bmp` neutral: grounded, low hum.
- `jam_s7_1.bmp` blink: optic blink.
- `jam_s7_2.bmp` happy: triumphant heavy stomp, sparks.
- `jam_s7_3.bmp` angry: connectors charging red, ground-cracking charge.

### jam_s9 — Ultimate · *Radio Behemoth*
> An immense mechanical demon whose roar distorts the air, concentric red
> shockwave rings, glowing reactor chest, broken antenna crown.
- `jam_s9_0.bmp` neutral: looming, faint shockwave.
- `jam_s9_1.bmp` blink: reactor glow dips.
- `jam_s9_2.bmp` happy: roaring with pride, bright rings.
- `jam_s9_3.bmp` angry: full roar, violent red shockwaves, reactor overload.

### jam_s10 — APEX · *Noise Leviathan*
> A colossal digital kaiju covered in broken antennas and red reactors, analog
> static crawling over its body, apocalyptic red glow.
- `jam_s10_0.bmp` neutral: towering, static shimmer.
- `jam_s10_1.bmp` blink: reactors pulse off.
- `jam_s10_2.bmp` happy: reactors blaze, antennas crackle gleefully.
- `jam_s10_3.bmp` angry: maximum static storm, every reactor firing red.

---

## 🟣 SPAMMER — "Cyber-Psycho" (Troll / BLE Spam)
**Palette:** magenta, acid green, glitch art. Asymmetry, retro error-window shapes.

### spa_s1 — Baby · *Corrupt Blob*
> A magenta pixel blob constantly shifting shape, glitchy edges, one lopsided eye,
> acid-green flecks.
- `spa_s1_0.bmp` neutral: wobbling blob.
- `spa_s1_1.bmp` blink: eye glitches out briefly.
- `spa_s1_2.bmp` happy: bouncing, broken-file icons orbiting, grin.
- `spa_s1_3.bmp` angry: violent glitch tearing, jagged magenta spikes.

### spa_s3 — In-Training · *Pop-up*
> A creature shaped like an old OS error window, many mismatched eyes, title-bar
> "mouth", horizontal screen-tearing, ribbon cables dangling.
- `spa_s3_0.bmp` neutral: idle, mild tearing.
- `spa_s3_1.bmp` blink: all eyes blink out of sync.
- `spa_s3_2.bmp` happy: eyes spiral happily, green sparkles.
- `spa_s3_3.bmp` angry: window cracks, red error glyphs, aggressive tearing.

### spa_s5 — Rookie · *Digital Voodoo Doll*
> A crooked humanized figure, one arm longer than the other, the other arm a
> bundle of cables, sudden palette swaps, ghost notifications swarming.
- `spa_s5_0.bmp` neutral: lopsided stand, faint swarm.
- `spa_s5_1.bmp` blink: stitched eyes blink.
- `spa_s5_2.bmp` happy: cables wave, notification swarm sparkles green/magenta.
- `spa_s5_3.bmp` angry: palette glitch-strobe, cables lash out.

### spa_s7 — Champion · *Amalgam*
> A monster built from broken CRT monitors and melted keyboards, RGB chromatic
> aberration splitting its body, dense static aura.
- `spa_s7_0.bmp` neutral: humming CRTs, mild aberration.
- `spa_s7_1.bmp` blink: every CRT shows a blink frame.
- `spa_s7_2.bmp` happy: screens flash smiley faces, green static glee.
- `spa_s7_3.bmp` angry: heavy RGB split, screens flash error, lunging.

### spa_s9 — Ultimate · *Botnet Legion*
> A tornado of digital garbage with shifting faces churning inside, magenta/green
> debris, multiple after-image trails.
- `spa_s9_0.bmp` neutral: slow swirling column.
- `spa_s9_1.bmp` blink: inner faces blink together.
- `spa_s9_2.bmp` happy: faces grin, debris sparkles, faster swirl.
- `spa_s9_3.bmp` angry: violent vortex, screaming faces, debris flung out.

### spa_s10 — APEX · *Fatal Error (BSOD)*
> A lovecraftian deity whose tentacles are made of corrupted source code, simulated
> blue-screen flashes radiating, magenta/green corruption.
- `spa_s10_0.bmp` neutral: looming, faint BSOD flicker.
- `spa_s10_1.bmp` blink: many eyes blink in waves.
- `spa_s10_2.bmp` happy: gleeful BSOD bloom, code tentacles curl.
- `spa_s10_3.bmp` angry: full blue-screen storm, tentacles strike, glitch overload.

---

## 🔵 SNIFFER — "El Operativo" (Recon / PCAP Capture)
**Palette:** dark blue, dark chrome, tactical-tech. Aerodynamic / insectoid, radars.

### sni_s1 — Baby · *Floating Lens*
> A miniature spherical surveillance camera drone, single glowing blue lens, a thin
> scanning light beam sweeping below it, dark chrome shell.
- `sni_s1_0.bmp` neutral: hovering, slow scan beam.
- `sni_s1_1.bmp` blink: lens iris closes.
- `sni_s1_2.bmp` happy: lens widens bright, cheerful tilt, sparkle.
- `sni_s1_3.bmp` angry: lens glows red-blue, beam snaps to a target lock.

### sni_s3 — In-Training · *Micro-Drone*
> A small robot with 4 mechanical spider legs and a rotating radar dish on top,
> blue pulsing radar rings, chrome body.
- `sni_s3_0.bmp` neutral: legs planted, radar pulsing.
- `sni_s3_1.bmp` blink: sensor eye blink.
- `sni_s3_2.bmp` happy: legs perky, radar pings rapidly, sparkle.
- `sni_s3_3.bmp` angry: legs braced, radar locks red, aggressive.

### sni_s5 — Rookie · *Tracker*
> A bipedal insectoid operative, face a single blue visor light (Splinter-Cell
> style goggles), sleek carbon limbs, smooth stealthy posture.
- `sni_s5_0.bmp` neutral: crouched ready, visor soft.
- `sni_s5_1.bmp` blink: visor flicker.
- `sni_s5_2.bmp` happy: visor curves up, light hop, sparkle.
- `sni_s5_3.bmp` angry: visor flares, blade-ready stance, ping burst.

### sni_s7 — Champion · *Packet-Hunter*
> A mechanical wolf/feline with carbon-fiber plating, blue ping waves emitting when
> it detects a target, sleek predatory build.
- `sni_s7_0.bmp` neutral: prowling, faint pings.
- `sni_s7_1.bmp` blink: eyes blink.
- `sni_s7_2.bmp` happy: tail-up, bright ping rings, playful.
- `sni_s7_3.bmp` angry: fangs bared, red-blue pings, pounce.

### sni_s9 — Ultimate · *Predator Satellite*
> A living low-orbit craft with data-interception cannons and targeting reticles,
> dark blue hull, projected aiming grids.
- `sni_s9_0.bmp` neutral: hovering, reticles idle.
- `sni_s9_1.bmp` blink: sensor array dims.
- `sni_s9_2.bmp` happy: reticles spin cheerfully, blue bloom.
- `sni_s9_3.bmp` angry: cannons charge, reticles lock red, firing pose.

### sni_s10 — APEX · *The Eye (Omniscient)*
> A colossal central server tower ringed by rotating satellites, a giant all-seeing
> blue eye, cascading matrix code rain around it.
- `sni_s10_0.bmp` neutral: rings rotating, eye watching.
- `sni_s10_1.bmp` blink: the great eye blinks.
- `sni_s10_2.bmp` happy: rings accelerate, matrix rain sparkles.
- `sni_s10_3.bmp` angry: eye glares red, satellites lock, data storm.

---

## 🟡 STRIKER — "El Interceptor" (Red Team / WiFi Deauth)
**Palette:** electric yellow, black. Sporty / shōnen, pointed lightning shapes.

### str_s1 — Baby · *Static Fluff*
> A small crunchy yellow fluffball with random electric arcs popping off it, big
> spark eyes, black accents.
- `str_s1_0.bmp` neutral: idle, occasional arc.
- `str_s1_1.bmp` blink: eyes shut, arcs settle.
- `str_s1_2.bmp` happy: bouncing, crackling sparks, grin.
- `str_s1_3.bmp` angry: fur spiked, fierce arcs, snarl.

### str_s3 — In-Training · *Electric Bird*
> A jumpy bird-like creature with springy legs and pointed heat-sink wings, leaving
> a yellow electric trail when it hops.
- `str_s3_0.bmp` neutral: perched, small sparks.
- `str_s3_1.bmp` blink: eyes blink.
- `str_s3_2.bmp` happy: mid-hop with bright trail, cheerful.
- `str_s3_3.bmp` angry: wings flared sharp, crackling dive.

### str_s5 — Rookie · *WiFi Ninja*
> A stylized agile ninja figure using braided cables as a scarf, trembling with
> stored energy, yellow sparks at the fingertips.
- `str_s5_0.bmp` neutral: ready stance, faint tremor.
- `str_s5_1.bmp` blink: mask-eyes blink.
- `str_s5_2.bmp` happy: spinning data-shurikens, confident grin.
- `str_s5_3.bmp` angry: charged strike pose, fierce yellow arcs.

### str_s7 — Champion · *Plasma Knight*
> An elegant pointed-armor knight wielding a giant directional antenna as a
> lance/sword, yellow lightning arcing between limbs, Wi-Fi-shaped energy wings.
- `str_s7_0.bmp` neutral: guard stance, soft arcs.
- `str_s7_1.bmp` blink: visor blink.
- `str_s7_2.bmp` happy: lance raised triumphantly, bright wings.
- `str_s7_3.bmp` angry: lance thrust, full lightning storm, blazing wings.

### str_s9 — Ultimate · *Storm Lord*
> A levitating figure at the center of orbiting "signal spheres", visible EMP pulses
> distorting the background, crackling yellow aura.
- `str_s9_0.bmp` neutral: floating, spheres orbit.
- `str_s9_1.bmp` blink: aura dims.
- `str_s9_2.bmp` happy: spheres spin bright, joyful surge.
- `str_s9_3.bmp` angry: EMP burst, spheres weaponized, lightning barrage.

### str_s10 — APEX · *Quasar*
> A being of pure blinding energy, barely any physical features, high-voltage
> strobe flashes radiating outward, white-yellow core.
- `str_s10_0.bmp` neutral: steady radiant core.
- `str_s10_1.bmp` blink: core pulse dip.
- `str_s10_2.bmp` happy: harmonic flares, bright bloom.
- `str_s10_3.bmp` angry: violent strobe discharge, jagged voltage beams.
