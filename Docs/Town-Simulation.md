# Signal Grove town simulation

Signal Grove contains a deterministic, data-driven large-town simulation. A supplied seed creates a westward connected street graph, 52 lots, 49 enterable furnished buildings, 20 named businesses/job sites, 100 assigned resident beds, and every required query destination. A batched world subsystem advances exactly 100 residents through travel, needs, household relationships, reservations, utility decisions, hourly work, thoughts, and money transactions without running a decision tick on each NPC Actor. The player owns the same four needs and money state and can perform every configured NPC activity.

## Run and inspect

Launch Signal Grove normally. No additional editor placement is required: `AUEGT1WorldDirector` ensures the authored `AUEGT1Town`, and the town creates and registers its destination components at `BeginPlay`.

- `F3` opens region and simulation diagnostics.
- `M` opens or closes the full-island map. The map shows sampled terrain, connected streets, all 52 venues, the player, Sanctuary, three Waystones and trails, plus a numbered directory for every non-residential place.
- `F4` cycles the inspected resident.
- `F5` saves the generated seed, clock, venues, reservations, metrics, and all resident state.
- `F6` restores that state and regenerates the matching town visuals.
- Aim at an interior activity marker and press `E` to sleep, cook/eat, shower, socialize, or work. Prompts report prices, closure reasons, job title, business, and hourly wage.

The always-visible top-right panel reports calendar date/time, local temperature, player money, all four player needs, and the latest activity result. The inspector reports time, seed, resident identity, needs, funds, home, unique bed, named housemates and whether they are family/friends/roommates, preferred job and hourly wage, personal money/session goals, work history, current thought, action, destination, utility scores, and the latest failure/replan reason.

The defaults live under `/Script/UEGT1.UEGT1TownSimulationSettings` in `Config/DefaultGame.ini`. They are also exposed through **Project Settings → Game → Signal Grove Town Simulation**. Runtime overrides are available for repeatable experiments:

```text
-UEGT1TownSeed=7319 -UEGT1TownNPCs=100
```

Population is clamped to at least 100. `SimMinutesPerRealSecond` controls the live clock. Automation advances the same model directly, so a two-day test does not need to wait two real days.

## Generation contract

`UEGT1TownGeneration::Generate` is the single source for the town layout. The same result drives HISM presentation and `UUEGT1TownDestinationComponent` query registration.

- Eighteen streets make a connected fifty-six-intersection graph stretching from X=-22,500 cm to X=4,700 cm. The original town-biome spine remains unchanged, so some new jobs sit in the westward outskirts instead of extending the urban biome over the farmland.
- Sidewalk strips flank every street; seeded grass lots and building dimensions fill the blocks.
- Every non-park lot is a collision-shell building with a walkable doorway, open-plan floor, kitchen, bathroom/toilet, shower, bed, roof skylight, and range-limited shadowless interior lamp. Homes contain four resident beds; other building types contain an amenity bed.
- Twenty-five homes each generate four visible bed-frame and bedding instances. The model assigns all 100 residents a unique bed in their assigned home and refuses to initialize if the bed invariant cannot be met.
- Two food venues, twenty job sites, two social venues, and three free parks provide capacity for the larger population.
- Every lot owns exterior access, entrance, interior-entry, kitchen, bathroom, shower, and activity points. A resident route exits through the current interior entry and door, follows connected street intersections, enters the destination door, and terminates at the action-specific bed/kitchen/shower/work/social point. Same-building actions stay inside. Automation checks every generated inter-venue route against every unrelated building footprint.
- The town spawns 102 query-only activity station Actors: three per home, one per food/work/social venue, and one per park. Repeated furnishing and shell geometry remains HISM-backed.
- Candidate lots preserve the three sanctuary-to-Waystone trail corridors; the town-biome and clutter-reservation spine extend west with the streets.

Keep new destinations in the generated layout. Do not add a visual-only venue that NPC queries cannot discover.

## Simulation ownership

`FUEGT1TownSimulationModel` owns pure state and deterministic decisions. It can run without a `UWorld`, which keeps automation fast and allows off-screen simulation to remain cheap. `UUEGT1TownSimulationSubsystem` owns the world clock, batches model updates four times per real second, evaluates the pure `UEGT1DayNight` lighting curve, moves the atmosphere sun, and coordinates its physical lux, color, shadow state, real-time skylight intensity/tint, fog color, and deterministic exposure. The curve reaches 65,000 lux and EV100 12.8 at noon, zero direct sunlight with a dim 0.12 cool skylight after dusk, and EV100 5.5 at midnight so the player can adapt to silhouettes without making the night read as day. The subsystem also performs SaveGame serialization. Lightweight resident Actors interpolate position and facing every rendered frame between those authoritative samples, so batching never appears as teleporting and does not introduce per-NPC decision ticks. Their configured 440 cm/s pace is about 8% below the normal player's 480 cm/s walk.

The planner scores configured action definitions using:

- need urgency and expected recovery;
- work/sleep schedule alignment;
- street-path travel distance;
- opening hours, reachability, and reserved capacity;
- price, available funds, and expected wages;
- assigned home or job preference.

A destination is reserved before travel. If it becomes closed, full, missing, unreachable, or cannot remain open long enough to finish the action, the reservation is released and the resident replans with a recorded reason. Restaurant meals and social venues have lower-cost alternatives: cooking at home costs `$1` for groceries and parks remain free. A resident with less than `$1` strongly prioritizes earning enough to eat.

Work has no fixed shift. Every resident has a preferred profession, savings goal, work drive, and personally acceptable session length, but may select any job with room. They commit to one hour, receive that site's hourly wage only after completing the hour, then run the full utility planner again. Falling needs, reaching the savings goal, reaching the personal session limit, closing time, or a better activity can all end the session. Capacity is released between hours. Work need costs are tuned per hour rather than as a four-hour lump.

The twenty default roles and their physical businesses are:

- Day jobs: Baker at Sunrise Bakery; Barista at Grove Cafe; Dockworker at Signal Harbor Freight; Groundskeeper at Parks Department; Carpenter at Westwood Workshop; Mechanic at Waystone Auto Works; Teacher at Signal Grove School; Office Clerk at Hearthstone Accounting; Courier at Grove Parcel Depot; Clinic Assistant at Signal Grove Clinic; Restaurant Cook at Lantern Restaurant; and Evening Server at Moonrise Restaurant.
- Limited jobs: Librarian at Town Library; Museum Guide at Heritage Museum; Market Vendor at Grove Grocery Market; and Florist at Wildflower Shop, all open 10:00–16:00.
- Overnight jobs: Night Watch at SecureTown Services (22:00–06:00) and Night Nurse at Nightingale Hospital (20:00–06:00).
- 24-hour jobs: Emergency Dispatcher at Emergency Services Center and Transit Operator at Town Transit Depot.

Hourly rates, capacities, and every operating window are `JobDefinitions` in `DefaultGame.ini` and are copied onto generated job destinations. Opening is inclusive, closing is exclusive, overnight windows wrap midnight, and a planned hour must fit before closing.

Each four-bed home receives a deterministic household type. Every pair sharing that home gets an explicit, symmetric `Family`, `Friend`, or `Roommate` relationship; no co-resident is left ambiguous. Relationships, player needs/money/activity history, calendar time, venue interiors, and routes are stored in SaveGame version 4.

Player activities call the pure simulation model rather than applying presentation-only effects. Time advances by the configured duration, so the player and every NPC decay needs together; completion then applies the same action effects and prices. Player work is a one-hour decision paid at the selected business's hourly rate. Temperature is derived from the player's sampled biome climate plus a diurnal cycle.

The model writes a short current thought whenever a resident plans, travels, arrives, works, rests, eats, maintains hygiene, or socializes. The HUD projects those motivations above the corresponding interpolated resident positions as chat bubbles. To control clutter and cost, only the nearest twelve residents within 3,000 cm are visible; Actors remain presentation-only and do not gain decision ticks or widget render targets.

Action definitions, prices, job definitions, hourly wages, durations, effects, need decay, thresholds, venue hours/capacities, bubble range/count, citizen walking speed, and clock speed are config data. Add a new tuned behavior as another action definition and corresponding model rule rather than placing constants in an NPC Actor.

## Verification

Run:

```powershell
./Scripts/Build.ps1
./Scripts/Test.ps1
./Scripts/Smoke-Gameplay.ps1
```

Tests under `UEGT1.TownSimulation.*` cover deterministic large-town generation, every-building amenity placement, door ordering, unrelated-building path avoidance, player/NPC activity parity, calendar behavior, twenty named businesses and all requested hour categories, `$1` groceries, hourly pay/session accounting, explicit symmetric households, one hundred unique resident bed assignments, needs, transactions, replanning, SaveGame serialization, interpolation, and a deterministic two-day run with 100 residents. The gameplay smoke requires 49 interiors, 102 activity stations, and the runtime 100-resident/100-bed/20-job/relationship contract in addition to the 154-tile World Partition signals.

## Extension boundary and current limits

Destination components and resident state expose Blueprint-friendly data. Namespaced `ExtensionValues` maps on venues and residents provide a serialization-safe attachment point for later inventories, relationships, skills, business ownership, vehicles, or crime modules without making the current planner own those systems.

Residents currently use deterministic door/street-graph travel with frame-interpolated procedural visuals, not CharacterMovement/NavMesh locomotion, crowd avoidance, or skeletal animation. Interiors and furnishings are procedural low-poly open-plan spaces rather than hand-authored rooms. Venue capacity remains a reservation count rather than one physical slot per visitor. Job sites pay configured hourly wages directly; businesses do not yet own accounts, stock, pricing strategy, or a macroeconomy. Thought text reflects simulation state through authored templates rather than a generative dialogue service. Multiplayer is outside this milestone.
