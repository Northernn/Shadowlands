

enum StepScenario
{
    STEP_1 = 5418,
    STEP_2 = 5464,

    //[2] PickedStep : 5420
    //[3] PickedStep : 5542
};

enum ActionAndEvent
{
    ACTION_START_WARTHION_MOVE = 1,
    ACTION_IN_COMBAT           = 2,
    ACTION_PREDATOR_PRESENCE  = 3,
    ACTION_START_EVENT_LORAC  = 4,
    ACTION_WARTHION_FLY = 5,
};

enum SpellsScenario
{
   SPELL_CAST_EARTH_WARDERS_PROTECTION  = 410728,
   AURA_EARTH_WARDERS_PROTECTION        = 406600,
   SPELL_CONVERSATION_DISCUSS_THE_PLAN  = 404463,
   SPELL_CONVERSATION_SABELLIAN_INTRO   = 404410,

   SPELL_WRATHION_INITIAL_TRANSFORM  = 373400,
   SPELL_SABELLIAN_INITIAL_TRANSFORM = 374008,

   //(Cast) SpellID: 412583 (Predator Presence)
    //(Cast) SpellID: 412418 (Throw Boulder)
    //(Cast) SpellID: 412416 (Throw Boulder)
    //(Cast) SpellID: 406669 (Throw Boulder) -- Este es el Spell que Castea el Mostruo a sabellian
    // (Cast) SpellID: 407131 (Throw Boulder)
    //[0] SpellID: 412582 (Predatory Fear)
    //(Cast) SpellID: 404325 (S1 - Battle RP Starts, Companions Conversation)
    //[0] SpellID: 412571 (Knockdown Stun)
    //(Cast) SpellID: 373043 (Wrathion Transform)
    //(Cast) SpellID: 372334 (Wrathion Transform)
    //(Cast) SpellID: 373342 (Wrathion Transform)
    //[0] SpellID: 373343 (Growth)
   // (Cast)SpellID: 406706 (Strafe) Spell que castea de fuego Wrathion
    //(Cast) SpellID: 406784 (Strafe)
    //SpellID: 411850 ([DNT] Spell) Spell que mataa los npc cuando wrathio hecha fuego

// 201412 waypoint donde wrathion sale volando Flags: 2438990336 (Flying, Catmullrom, CanSwim, UncompressedPath, Unknown5, Steering, Unknown10)
   //(MovementMonsterSpline)(MovementSpline)[0] Points: X: -40.37847 Y : 774.2118 Z : 83.34778
   //(MovementMonsterSpline)(MovementSpline)[1] Points : X : -57.42882 Y : 802.4792 Z : 87.91718
   //(MovementMonsterSpline)(MovementSpline)[2] Points : X : -86.31944 Y : 851.0764 Z : 77.22843
   //(MovementMonsterSpline)(MovementSpline)[3] Points : X : -98.58854 Y : 882.2899 Z : 68.3041
};

enum CreaturesID
{
    NPC_WARTHION_INTRO = 201412,
    NPC_SABELLIAN_INTRO = 201411,
    NPC_MONSTROUS_MUD_INTRO = 202593,

};
/*
Waypoint Moustro

[0] WayPoints: X: -64.50174 Y: 772.2908 Z: 79.13559
[1] WayPoints: X: -66.25174 Y: 774.5408 Z: 78.88559
[2] WayPoints: X: -66.50174 Y: 775.2908 Z: 78.88559
[3] WayPoints: X: -67.25174 Y: 777.0408 Z: 78.88559
[4] WayPoints: X: -67.50174 Y: 778.2908 Z: 79.63559
[5] WayPoints: X: -67.75174 Y: 778.7908 Z: 79.88559
[6] WayPoints: X: -67.50174 Y: 782.5408 Z: 78.88559
[7] WayPoints: X: -67.50174 Y: 783.5408 Z: 78.63559
[8] WayPoints: X: -68.00174 Y: 788.5408 Z: 78.38559
[9] WayPoints: X: -68.25174 Y: 790.5408 Z: 77.88559
[10] WayPoints: X: -68.50174 Y: 791.5408 Z: 77.63559
[11] WayPoints: X: -68.50174 Y: 792.0408 Z: 77.88559
[12] WayPoints: X: -68.50174 Y: 793.0408 Z: 77.63559
[13] WayPoints: X: -68.50174 Y: 794.7908 Z: 77.38559
[14] WayPoints: X: -68.50174 Y: 795.7908 Z: 76.63559
[15] WayPoints: X: -68.75174 Y: 796.7908 Z: 76.13559
[16] WayPoints: X: -69.00174 Y: 797.7908 Z: 75.88559
[17] WayPoints: X: -69.50174 Y: 799.7908 Z: 75.88559
[18] WayPoints: X: -69.50174 Y: 800.0408 Z: 76.38559
[19] WayPoints: X: -70.00174 Y: 802.0408 Z: 75.88559
[20] WayPoints: X: -70.00174 Y: 803.0408 Z: 75.63559
[21] WayPoints: X: -70.00174 Y: 804.0408 Z: 75.38559
[22] WayPoints: X: -69.50174 Y: 805.0408 Z: 75.13559
[23] WayPoints: X: -68.75174 Y: 805.7908 Z: 74.13559
[24] WayPoints: X: -68.50174 Y: 806.2908 Z: 74.13559
[25] WayPoints: X: -68.00174 Y: 807.0408 Z: 73.63559
[26] WayPoints: X: -67.50174 Y: 808.0408 Z: 73.88559
[27] WayPoints: X: -67.50174 Y: 808.2908 Z: 73.88559
[28] WayPoints: X: -68.00174 Y: 810.2908 Z: 73.63559
[29] WayPoints: X: -68.50174 Y: 812.5408 Z: 73.38559
[30] WayPoints: X: -68.50174 Y: 815.5408 Z: 73.13559
[31] WayPoints: X: -68.50174 Y: 816.7908 Z: 73.13559

*/
Position const wrathionmove[1] =
{
    {-8.366320f, 755.195984f, 91.987503f, 0},
};

