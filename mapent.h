// * This information should come from somewhere outside the rendering
// * engine.  I think Q3A puts it in one of the .qvm files...
// * So, these values are just guesses.
   

// mapent

#define DEFBOB 5.0f, 1.0f
#define DEFNOBOB 0.0f, 0.0f
#define DEFROT 180.0f
#define DEFSCALE 1.0f


/// Quake III Arena Entities

{"weapon_grenadelauncher", 0, 1,
 {{"models/weapons2/grenadel/grenadel.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_shotgun", 0, 1,
 {{"models/weapons2/shotgun/shotgun.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_plasmagun", 0, 1,
 {{"models/weapons2/plasma/plasma.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_railgun", 0, 1,
 {{"models/weapons2/railgun/railgun.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_lightning", 0, 1,
 {{"models/weapons2/lightning/lightning.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_rocketlauncher", 0, 1,
 {{"models/weapons2/rocketl/rocketl.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"weapon_bfg", 0, 1,
 {{"models/weapons2/bfg/bfg.md3", DEFROT, DEFBOB, 1.5f, 0}, },
 },

{"ammo_bullets", 0, 1,
 {{"models/powerups/ammo/machinegunam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },

{"ammo_shells", 0, 1,
 {{"models/powerups/ammo/shotgunam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },

{"ammo_cells", 0, 1,
 {{"models/powerups/ammo/plasmaam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },

{"ammo_slugs", 0, 1,
 {{"models/powerups/ammo/railgunam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },

{"ammo_lightning", 0, 1,
 {{"models/powerups/ammo/lightningam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },

{"ammo_rockets", 0, 1,
 {{"models/powerups/ammo/rocketam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },    

{"ammo_bfg", 0, 1,
 {{"models/powerups/ammo/bfgam.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },    

{"item_enviro", 0, 2,
 {{"models/powerups/instant/enviro.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/enviro_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },       

{"item_armor_body", 0, 1,
 {{"models/powerups/armor/armor_red.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },    

{"item_armor_combat", 0, 1,
 {{"models/powerups/armor/armor_yel.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },    

{"item_armor_shard", 0, 1,
 {{"models/powerups/armor/shard.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },  

{"item_health_mega", 0, 2,
 {{"models/powerups/health/mega_cross.md3", 2.0*DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/health/mega_sphere.md3", 0.0f, DEFBOB,  DEFSCALE, 0} },
 },       

{"item_health_large", 0, 2,
 {{"models/powerups/health/large_cross.md3", 2.0*DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/health/large_sphere.md3", 0.0f, DEFBOB,  DEFSCALE, 0} },
 },       

{"item_health", 0, 2,
 {{"models/powerups/health/medium_cross.md3", 2.0*DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/health/medium_sphere.md3", 0.0f, DEFBOB,  DEFSCALE, 0} },
 },       

{"item_health_small", 0, 2,
 {{"models/powerups/health/small_cross.md3", 2.0*DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/health/small_sphere.md3", 0.0f, DEFBOB,  DEFSCALE, 0} },
 },       

{"item_quad", 0, 2,
 {{"models/powerups/instant/quad.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/quad_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },    
 
{"item_haste", 0, 2,
 {{"models/powerups/instant/haste.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/haste_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },    
 
{"item_flight", 0, 2,
 {{"models/powerups/instant/flight.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/flight_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },    
 
{"item_regen", 0, 2,
 {{"models/powerups/instant/regen.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/regen_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },    
 
{"item_invis", 0, 2,
 {{"models/powerups/instant/invis.md3", DEFROT, DEFBOB, DEFSCALE, 0},
  {"models/powerups/instant/invis_ring.md3", -2.0*DEFROT, DEFNOBOB, 1.1f, 10.0f} },
 },    
 
{"holdable_teleporter", 0, 1,
 {{"models/powerups/holdable/teleporter.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       



// Voyager Elite Force Entities


{"item_hypo", 0, 1,
 {{"models/powerups/trek/hypo_double.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },    
    
{"item_hypo_small", 0, 1,
 {{"models/powerups/trek/hypo_single.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"item_armor_combat", 0, 1,
 {{"models/powerups/trek/armor.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"item_armor_body", 0, 1,
 {{"models/powerups/trek/armor2.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"item_armor_combat", 0, 1,
 {{"models/powerups/trek/armor3.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"item_regen", 0, 1,
 {{"models/powerups/trek/regen.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },   
 
{"item_invis", 0, 1,
 {{"models/powerups/trek/invisible.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },   
 
{"item_haste", 0, 1,
 {{"models/powerups/trek/haste.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },   
 
{"item_enviro", 0, 1,
 {{"models/powerups/trek/enviro.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },   
 
{"item_quad", 0, 1,
 {{"models/powerups/trek/quad_damage.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },   
  
{"item_armor_shard", 0, 1,
 {{"models/powerups/trek/armor_shard.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"weapon_imod", 0, 1,
 {{"models/weapons2/imod/imod2_w.md3", DEFROT, DEFBOB,  1.5f, 0}, },
 },       

{"weapon_compressionrifle", 0, 1,
 {{"models/weapons2/prifle/prifle_w.md3", DEFROT, DEFBOB,  1.5f, 0}, },
 },       

{"weapon_tetriondisruptor", 0, 1,
 {{"models/weapons2/tpd/tpd_w.md3", DEFROT, DEFBOB,  1.5f, 0}, },
 },       

{"weapon_scavenger", 0, 1,
 {{"models/weapons2/scavenger/scavenger_w.md3", DEFROT, DEFBOB,  1.5f, 0}, },
 },       


{"ammo_tetriondisruptor", 0, 1,
 {{"models/powerups/trek/tetrion_ammo.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"ammo_imod", 0, 1,
 {{"models/powerups/trek/imod_ammo.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"ammo_scavenger", 0, 1,
 {{"models/powerups/trek/scavenger_ammo.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       

{"ammo_compressionrifle", 0, 1,
 {{"models/powerups/trek/prifle_ammo.md3", DEFROT, DEFBOB, DEFSCALE, 0}, },
 },       



{NULL} // Sentinel 
