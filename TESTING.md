# Testa stabilitetsbygget (1.10.50)

Det här är testerna under "Definition av klart". De kräver spelet och måste köras på riktigt.
Alla spelare måste ha **samma** `crests_of_courage.dusk` (version 1.10.50). Andra versioner nekas
vid anslutning ("refusing ... version").

Dusklight: release-noterna för 1.10.0 anger Dusklight **2.0.3**. Uppdatera till den.

## Debuginställningar i ett release-bygge

Release-bygget stänger av alla `debug_*`-inställningar som finns kvar i `config.json` och visar dem i
rutan "Problems at startup". Vill du köra testerna, starta Dusklight med miljövariabeln
`COOP_ALLOW_DEBUG=1`. Sätt sedan inställningarna med `--cvar`. Konfignyckeln är
`mod.dev_remiafterdark_coop__mod.<namn>`.

Windows (PowerShell), från Dusklight-mappen:

```powershell
$env:COOP_ALLOW_DEBUG = "1"
.\dusklight.exe --cvar mod.dev_remiafterdark_coop__mod.debug_selftest=1 `
                --cvar mod.dev_remiafterdark_coop__mod.debug_world_selftest_ticks=600 `
                --cvar mod.dev_remiafterdark_coop__mod.debug_enemy_selftest_ticks=300 `
                --cvar mod.dev_remiafterdark_coop__mod.debug_spawn_bomb_ticks=900
```

Kör samma sak på båda datorerna. Den ena ska vara värd och den andra ansluta. Stäng av
debuginställningarna efteråt: starta utan `COOP_ALLOW_DEBUG`, så stängs de av automatiskt.

## 1. Selftest i 30 minuter (fiende, värld, spawn och soak)

1. Starta båda med inställningarna ovan, anslut och gå in i spelet.
2. Låt det stå i minst 32 minuter.
3. Sök i loggen efter `[SELFTEST]`. Varje rad `T1`–`T4` och `T9` ska säga `PASS`.
   `T9` är soak-testet. Det loggar minnet var femte minut och ger till sist PASS eller FAIL för
   minnestillväxt, frigöringskö och nya problem.
4. Inga rader med `[ISSUE]` ska tillkomma under körningen.

## 2. Modellbyten, 50 gånger

`debug_skin_cycle_ticks=N` byter till nästa modell i `res/models/` var N:e tick, och efter den sista
tillbaka till Link. `debug_outfit_cycle_ticks=N` byter dräkt, och varg om man är varg.

```powershell
.\dusklight.exe --cvar mod.dev_remiafterdark_coop__mod.debug_skin_cycle_ticks=120
```

Med 8 modeller plus Link blir 50 byten ungefär 6 varv, alltså cirka 100 sekunder per varv.
Kör det här:

- som Link på land,
- som varg (`debug_wolf=1` eller förvandla dig själv),
- i vatten (Zora-dräkten, under ytan),
- under warp. Låt den andra spelaren warpa med Ooccoo, och warpa själv under bytet.

Godkänt: ingen krasch. Ett byte under förvandling, warp, cutscene eller laddning väntar. Loggen
skriver då `[SKIN] model change held until ...`, och bytet görs direkt efteråt.

## 3. Genomspelning med två spelare

Från Ordon till och med Forest Temple, inklusive Ook och Diababa, och sedan Goron Mines. Ha
"Story progress" påslaget (standard). Håll koll på fliken **Network**: blocket "Sync" ska säga
"same as the host's" när ni är i samma område. Om något är fel, tryck **Resync the world now**
och notera vad som hände.

Spara `sync-misses.log` från moddens datamapp och skicka med den om något inte stämmer.

## 4. Disconnect och reconnect

Koppla ner nätverket eller stäng spelet hos den ena, och anslut igen i tre lägen:

- mitt i en bossfight (Diababa),
- mitt i en cutscene,
- i en twilight-del.

Godkänt: båda kan spela vidare. Bossdörren väntar högst 20 sekunder och kan startas direkt med
D-pad ned eller "Start the boss fight without me". En hållen cutscene släpper världsstaten efter
45 sekunder, och en röstning om att hoppa över går igenom efter 15 sekunder.
