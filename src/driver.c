#include "driver.h"


/* "Pacman hardware" games */
extern struct GameDriver pacman_driver;		/* (c) 1980 Midway */
extern struct GameDriver pacplus_driver;	/* hack */
extern struct GameDriver jrpacman_driver;	/* (c) 1983 Midway */
extern struct GameDriver pacmod_driver;		/* (c) 1981 Midway */
extern struct GameDriver namcopac_driver;	/* (c) 1980 Namco */
extern struct GameDriver pacmanjp_driver;	/* (c) 1980 Namco */
extern struct GameDriver hangly_driver;		/* hack */
extern struct GameDriver puckman_driver;	/* hack */
extern struct GameDriver piranha_driver;	/* hack */
extern struct GameDriver mspacman_driver;	/* (c) 1981 Midway (but it's a bootleg) */
extern struct GameDriver mspacatk_driver;	/* hack */
extern struct GameDriver crush_driver;		/* bootleg Make Trax */
extern struct GameDriver eyes_driver;		/* (c) 1982 Digitrex + "Rockola presents" */
extern struct GameDriver ponpoko_driver;	/* (c) 1982 Sigma Ent. Inc. */
extern struct GameDriver maketrax_driver;	/* (c) 1981 Williams */
extern struct GameDriver pengo_driver;		/* (c) 1982 Sega */
extern struct GameDriver pengoa_driver;		/* (c) 1982 Sega */
extern struct GameDriver penta_driver;		/* bootleg */

/* "Galaxian hardware" games */
extern struct GameDriver galaxian_driver;	/* (c) Namco */
extern struct GameDriver galmidw_driver;	/* (c) Midway */
extern struct GameDriver galnamco_driver;	/* hack */
extern struct GameDriver superg_driver;		/* hack */
extern struct GameDriver galapx_driver;		/* hack */
extern struct GameDriver galap1_driver;		/* hack */
extern struct GameDriver galap4_driver;		/* hack */
extern struct GameDriver galturbo_driver;	/* hack */
extern struct GameDriver pisces_driver;		/* ? */
extern struct GameDriver japirem_driver;	/* (c) Irem */
extern struct GameDriver uniwars_driver;	/* (c) Karateco (bootleg?) */
extern struct GameDriver warofbug_driver;	/* (c) 1981 Armenia */
extern struct GameDriver redufo_driver;		/* ? */
extern struct GameDriver pacmanbl_driver;	/* bootleg */
extern struct GameDriver mooncrst_driver;	/* (c) 1980 Nihon Bussan */
extern struct GameDriver mooncrsg_driver;	/* (c) 1980 Gremlin */
extern struct GameDriver mooncrsb_driver;	/* bootleg */
extern struct GameDriver fantazia_driver;	/* bootleg */
extern struct GameDriver eagle_driver;		/* (c) Centuri */
extern struct GameDriver moonqsr_driver;	/* (c) 1980 Nichibutsu */
extern struct GameDriver checkman_driver;	/* (c) 1982 Zilec-Zenitone */
extern struct GameDriver moonal2_driver;	/* Nichibutsu */
extern struct GameDriver moonal2b_driver;	/* Nichibutsu */

/* "Scramble hardware" (and variations) games */
extern struct GameDriver scramble_driver;	/* GX387 (c) 1981 Stern */
extern struct GameDriver atlantis_driver;	/* (c) 1981 Comsoft */
extern struct GameDriver theend_driver;		/* (c) 1980 Stern */
extern struct GameDriver ckongs_driver;		/* bootleg */
extern struct GameDriver froggers_driver;	/* bootleg */
extern struct GameDriver amidars_driver;	/* (c) 1982 Konami */
extern struct GameDriver triplep_driver;	/* (c) 1982 KKI */
extern struct GameDriver scobra_driver;		/* GX316 (c) 1981 Stern */
extern struct GameDriver scobrak_driver;	/* GX316 (c) 1981 Konami */
extern struct GameDriver scobrab_driver;	/* GX316 (c) 1981 Karateco (bootleg?) */
extern struct GameDriver losttomb_driver;	/* (c) 1982 Stern */
extern struct GameDriver rescue_driver;		/* (c) 1982 Stern */
extern struct GameDriver minefld_driver;	/* (c) 1983 Stern */
extern struct GameDriver anteater_driver;	/* (c) 1982 Tago */
extern struct GameDriver armorcar_driver;	/* (c) 1981 Stern */
extern struct GameDriver tazmania_driver;	/* (c) 1982 Stern */
extern struct GameDriver stratgyx_driver;	/* (c) 1981 Stern */
extern struct GameDriver stratgyb_driver;	/* bootleg (of the Konami version?) */
extern struct GameDriver darkplnt_driver;	/* (c) 1982 Stern */
extern struct GameDriver hustler_driver;	/* GX343 (c) 1981 Konami */
extern struct GameDriver pool_driver;
extern struct GameDriver frogger_driver;	/* GX392 (c) 1981 Sega */
extern struct GameDriver frogsega_driver;	/* GX392 (c) 1981 Sega */
extern struct GameDriver frogger2_driver;	/* GX392 (c) 1981 Sega */
extern struct GameDriver amidar_driver;		/* GX337 (c) 1982 Konami + Stern license */
extern struct GameDriver amidarjp_driver;	/* GX337 (c) 1981 Konami */
extern struct GameDriver amigo_driver;		/* bootleg */
extern struct GameDriver turtles_driver;	/* GX353 (c) 1981 Stern */
extern struct GameDriver turpin_driver;		/* GX353 (c) 1981 Sega */
extern struct GameDriver jumpbug_driver;	/* (c) 1981 Rock-ola */
extern struct GameDriver jbugsega_driver;	/* (c) 1981 Sega */
extern struct GameDriver flyboy_driver;		/* (c) 1982 Kaneko (bootleg?) */
extern struct GameDriver fastfred_driver;	/* (c) 1982 Atari */
extern struct GameDriver jumpcoas_driver;	/* (c) 1983 Kaneko */

/* "Crazy Climber hardware" games */
extern struct GameDriver cclimber_driver;	/* (c) 1980 Nihon Bussan */
extern struct GameDriver ccjap_driver;		/* (c) 1980 Nihon Bussan */
extern struct GameDriver ccboot_driver;		/* bootleg */
extern struct GameDriver ckong_driver;		/* (c) 1981 Falcon */
extern struct GameDriver ckonga_driver;		/* (c) 1981 Falcon */
extern struct GameDriver ckongjeu_driver;	/* bootleg */
extern struct GameDriver ckongalc_driver;	/* bootleg */
extern struct GameDriver monkeyd_driver;	/* bootleg */
extern struct GameDriver swimmer_driver;	/* (c) 1982 Tehkan */
extern struct GameDriver swimmera_driver;	/* (c) 1982 Tehkan */
extern struct GameDriver guzzler_driver;	/* (c) 1983 Tehkan */
extern struct GameDriver seicross_driver;	/* (c) 1984 Nichibutsu + Alice */
extern struct GameDriver friskyt_driver;

/* "Phoenix hardware" (and variations) games */
extern struct GameDriver phoenix_driver;	/* (c) 1980 Amstar */
extern struct GameDriver phoenixt_driver;	/* (c) 1980 Taito */
extern struct GameDriver phoenix3_driver;	/* T.P.N. */
extern struct GameDriver pleiads_driver;	/* (c) 1981 Centuri + Tehkan */
extern struct GameDriver pleitek_driver;	/* (c) 1981 Tehkan */
extern struct GameDriver naughtyb_driver;	/* (c) 1982 Jaleco + Cinematronics */
extern struct GameDriver popflame_driver;	/* (c) 1982 Jaleco */

/* Namco games */
extern struct GameDriver rallyx_driver;		/* (c) 1980 Midway */
extern struct GameDriver nrallyx_driver;	/* (c) 1981 Namco */
extern struct GameDriver locomotn_driver;	/* GX359 (c) 1982 Centuri + Konami license */
extern struct GameDriver jungler_driver;	/* GX327 (c) 1981 Konami */
extern struct GameDriver commsega_driver;	/* (c) 1983 Sega */
/* the following ones all have a custom I/O chip */
extern struct GameDriver bosco_driver;		/* (c) 1981 Midway */
extern struct GameDriver bosconm_driver;	/* (c) 1981 */
extern struct GameDriver galaga_driver;		/* (c) 1981 Midway */
extern struct GameDriver galaganm_driver;	/* (c) 1981 */
extern struct GameDriver galagabl_driver;	/* bootleg */
extern struct GameDriver gallag_driver;		/* bootleg */
extern struct GameDriver galagab2_driver;	/* bootleg */
extern struct GameDriver digdugnm_driver;	/* (c) 1982 */
extern struct GameDriver digdugat_driver;	/* (c) 1982 Atari */
extern struct GameDriver xevious_driver;	/* (c) 1982 + Atari license */
extern struct GameDriver xeviousn_driver;	/* (c) 1982 */
extern struct GameDriver sxevious_driver;	/* (c) 1984 */
extern struct GameDriver superpac_driver;	/* (c) 1982 Midway */
extern struct GameDriver superpcn_driver;	/* (c) 1982 */
extern struct GameDriver pacnpal_driver;	/* (c) 1983 */
extern struct GameDriver mappy_driver;		/* (c) 1983 */
extern struct GameDriver mappyjp_driver;	/* (c) 1983 */
extern struct GameDriver digdug2_driver;	/* (c) 1985 */
extern struct GameDriver todruaga_driver;	/* (c) 1984 */
extern struct GameDriver motos_driver;		/* (c) 1985 */
/* no custom I/O in the following, HD63701 (or compatible) microcontroller instead */
extern struct GameDriver pacland_driver;	/* (c) 1984 Midway */
extern struct GameDriver paclandn_driver;	/* (c) 1984 */
extern struct GameDriver paclanda_driver;	/* (c) 1984 */

/* Universal games */
extern struct GameDriver cosmica_driver;	/* (c) [1979] */
extern struct GameDriver panic_driver;		/* (c) 1980 */
extern struct GameDriver panica_driver;		/* (c) 1980 */
extern struct GameDriver ladybug_driver;	/* (c) 1981 */
extern struct GameDriver snapjack_driver;	/* (c) */
extern struct GameDriver cavenger_driver;	/* (c) 1981 */
extern struct GameDriver mrdo_driver;		/* (c) 1982 */
extern struct GameDriver mrdot_driver;		/* (c) 1982 + Taito license */
extern struct GameDriver mrlo_driver;		/* bootleg */
extern struct GameDriver mrdu_driver;		/* bootleg */
extern struct GameDriver docastle_driver;	/* (c) 1983 */
extern struct GameDriver docastl2_driver;	/* (c) 1983 */
extern struct GameDriver dounicorn_driver;	/* (c) 1983 */
extern struct GameDriver dowild_driver;		/* (c) 1984 */
extern struct GameDriver jjack_driver;		/* (c) 1984 */
extern struct GameDriver dorunrun_driver;	/* (c) 1984 */
extern struct GameDriver spiero_driver;		/* (c) 1987 */
extern struct GameDriver kickridr_driver;	/* (c) 1984 */

/* Nintendo games */
extern struct GameDriver dkong_driver;		/* (c) 1981 Nintendo of America */
extern struct GameDriver radarscp_driver;	/* (c) 1980 Nintendo */
extern struct GameDriver dkongjp_driver;	/* (c) 1981 Nintendo */
extern struct GameDriver dkongjr_driver;	/* (c) 1982 Nintendo of America */
extern struct GameDriver dkngjrjp_driver;	/* no copyright notice */
extern struct GameDriver dkjrjp_driver;		/* (c) 1982 Nintendo */
extern struct GameDriver dkjrbl_driver;		/* (c) 1982 Nintendo of America */
extern struct GameDriver dkong3_driver;		/* (c) 1983 Nintendo of America */
extern struct GameDriver mario_driver;		/* (c) 1983 Nintendo of America */
extern struct GameDriver masao_driver;		/* bootleg */
extern struct GameDriver hunchy_driver;		/* hacked Donkey Kong board */
extern struct GameDriver popeye_driver;
extern struct GameDriver popeyebl_driver;	/* bootleg - different hardware from the other ones */
extern struct GameDriver punchout_driver;	/* (c) 1984 - different hardware */
extern struct GameDriver spnchout_driver;

/* Midway 8080 b/w games */
extern struct GameDriver seawolf_driver;	/* 596 [1976] */
extern struct GameDriver gunfight_driver;	/* 597 [1975] */
/* 603 - Top Gun [1976] */
extern struct GameDriver tornbase_driver;	/* 605 [1976] */
extern struct GameDriver zzzap_driver;		/* 610 [1976] */
extern struct GameDriver maze_driver;		/* 611 [1976] */
extern struct GameDriver boothill_driver;	/* 612 [1977] */
/* 615 - Checkmate [1977] */
/* 618 - Road Runner [1977] */
/* 618 - Desert Gun [1977] */
/* 619 - Double Play [1977] */
/* 622 - Laguna Racer [1977] */
extern struct GameDriver gmissile_driver;	/* 623 [1977] */
/* 626 - M-4 [1977] */
extern struct GameDriver clowns_driver;		/* 630 [1978] */
/* 640 - Space Walk [1978] */
/* 642 - Extra Innings [1978] */
/* 643 - Shuffleboard [1978] */
/* 644 - Dog Patch [1978] */
extern struct GameDriver spcenctr_driver;	/* 645 [1980] */
/* 652 - Phantom II [1979] */
/* 730 - Bowling Alley [1979] */
extern struct GameDriver invaders_driver;	/* 739 [1979] */
/* 742 - Blue Shark [1980] */
extern struct GameDriver invdelux_driver;	/* 852 [1980] */
extern struct GameDriver invadpt2_driver;	/* 851 [1980] */
extern struct GameDriver earthinv_driver;
extern struct GameDriver spaceatt_driver;
extern struct GameDriver invrvnge_driver;
extern struct GameDriver galxwars_driver;
extern struct GameDriver lrescue_driver;
extern struct GameDriver desterth_driver;
extern struct GameDriver cosmicmo_driver;
extern struct GameDriver spaceph_driver;
extern struct GameDriver rollingc_driver;
extern struct GameDriver bandido_driver;
extern struct GameDriver astinvad_driver;
extern struct GameDriver schaser_driver;
extern struct GameDriver kamikaze_driver;
extern struct GameDriver lupin3_driver;

/* Bally Midway "Astrocade" games */
extern struct GameDriver wow_driver;		/* (c) 1980 */
extern struct GameDriver robby_driver;		/* (c) 1981 */
extern struct GameDriver gorf_driver;		/* (c) 1981 */
extern struct GameDriver gorfpgm1_driver;	/* (c) 1981 */
extern struct GameDriver seawolf2_driver;
extern struct GameDriver spacezap_driver;	/* (c) 1980 */
extern struct GameDriver ebases_driver;

/* Bally Midway "MCR" games */
/* MCR1 */
extern struct GameDriver solarfox_driver;	/* (c) 1981 */
extern struct GameDriver kick_driver;		/* (c) 1981 */
/* MCR2 */
extern struct GameDriver shollow_driver;	/* (c) 1981 */
extern struct GameDriver tron_driver;		/* (c) 1982 */
extern struct GameDriver kroozr_driver;		/* (c) 1982 */
extern struct GameDriver domino_driver;		/* (c) 1982 */
extern struct GameDriver wacko_driver;		/* (c) 1982 */
extern struct GameDriver twotiger_driver;	/* (c) 1984 */
/* MCR2 + MCR3 sprites */
extern struct GameDriver journey_driver;	/* (c) 1983 */
/* MCR3 */
extern struct GameDriver tapper_driver;		/* (c) 1983 */
extern struct GameDriver sutapper_driver;	/* (c) 1983 */
extern struct GameDriver rbtapper_driver;	/* (c) 1984 */
extern struct GameDriver dotron_driver;		/* (c) 1983 */
extern struct GameDriver destderb_driver;	/* (c) 1984 */
extern struct GameDriver timber_driver;		/* (c) 1984 */
extern struct GameDriver spyhunt_driver;	/* (c) 1983 */
extern struct GameDriver crater_driver;		/* (c) 1984 */
extern struct GameDriver rampage_driver;	/* (c) 1986 */
extern struct GameDriver sarge_driver;		/* (c) 1985 */
/* MCR 68000 */
extern struct GameDriver xenophob_driver;	/* (c) 1987 */
/* Power Drive - similar to Xenophobe? */

/* Irem games */
extern struct GameDriver mpatrol_driver;	/* (c) 1982 */
extern struct GameDriver mpatrolw_driver;	/* (c) 1982 + Williams license */
extern struct GameDriver mranger_driver;	/* bootleg */
extern struct GameDriver yard_driver;		/* (c) 1983 */
extern struct GameDriver vsyard_driver;		/* (c) 1983/1984 */
extern struct GameDriver kungfum_driver;	/* (c) 1984 */
extern struct GameDriver kungfub_driver;	/* bootleg */
extern struct GameDriver travrusa_driver;	/* (c) 1983 */
extern struct GameDriver motorace_driver;	/* (c) 1983 Williams license */
extern struct GameDriver lrunner_driver;	/* (c) 1984 + Broderbund license */
/* other games running on similar hardware: Kid Niki */

/* Gottlieb/Mylstar games (Gottlieb became Mylstar in 1983) */
extern struct GameDriver reactor_driver;	/* GV-100 (c) 1982 Gottlieb */
extern struct GameDriver mplanets_driver;	/* GV-102 (c) 1983 Gottlieb */
extern struct GameDriver qbert_driver;		/* GV-103 (c) 1982 Gottlieb */
extern struct GameDriver qbertjp_driver;	/* GV-103 (c) 1982 Gottlieb + Konami license */
extern struct GameDriver sqbert_driver;		/* (c) 1983 Mylstar - never released */
extern struct GameDriver krull_driver;		/* GV-105 (c) 1983 Gottlieb */
extern struct GameDriver mach3_driver;		/* GV-109 (c) 1983 Mylstar */
extern struct GameDriver usvsthem_driver;	/* GV-??? (c) 198? Mylstar */
extern struct GameDriver stooges_driver;	/* GV-113 (c) 1984 Mylstar */
extern struct GameDriver qbertqub_driver;	/* GV-119 (c) 1983 Mylstar */
extern struct GameDriver curvebal_driver;	/* GV-134 (c) 1984 Mylstar */

/* older Taito games */
extern struct GameDriver crbaloon_driver;	/* (c) 1980 Taito */
extern struct GameDriver crbalon2_driver;	/* (c) 1980 Taito */

/* Taito "Qix hardware" games */
extern struct GameDriver qix_driver;		/* (c) 1981 */
extern struct GameDriver qix2_driver;		/* (c) 1981 */
extern struct GameDriver sdungeon_driver;	/* (c) 1981 Taito America */
extern struct GameDriver zookeep_driver;	/* (c) 1982 Taito America */

/* Taito "Jungle King hardware" games */
extern struct GameDriver junglek_driver;	/* (c) 1982 */
extern struct GameDriver jhunt_driver;		/* (c) 1982 Taito America */
extern struct GameDriver elevator_driver;	/* (c) 1983 */
extern struct GameDriver elevatob_driver;	/* bootleg */
extern struct GameDriver wwestern_driver;	/* (c) 1982 */
extern struct GameDriver frontlin_driver;	/* (c) 1982 */
extern struct GameDriver alpine_driver;
extern struct GameDriver spaceskr_driver;	/* (c) 1981 */

/* other Taito games */
extern struct GameDriver bublbobl_driver;	/* (c) 1986 */
extern struct GameDriver boblbobl_driver;	/* bootleg */
extern struct GameDriver sboblbob_driver;	/* bootleg */
extern struct GameDriver rastan_driver;		/* (c) 1987 Taito Japan */
extern struct GameDriver rastsaga_driver;	/* (c) 1987 Taito */
extern struct GameDriver rainbow_driver;	/* (c) 1987 */
extern struct GameDriver rainbowe_driver;	/* (c) 1988 */
extern struct GameDriver arkanoid_driver;	/* (c) 1986 Taito */
extern struct GameDriver arknoidu_driver;	/* (c) 1986 Taito America */
extern struct GameDriver arkbl2_driver;		/* bootleg */
extern struct GameDriver arkabeta_driver;	/* bootleg */
extern struct GameDriver arkatayt_driver;	/* bootleg */
extern struct GameDriver superqix_driver;
extern struct GameDriver sqixbl_driver;		/* bootleg? but (c) 1987 */
extern struct GameDriver tnzs_driver;		/* (c) 1988 */
extern struct GameDriver tnzs2_driver;		/* (c) 1988 */
extern struct GameDriver slapfigh_driver;
extern struct GameDriver slapboot_driver;	/* bootleg */
extern struct GameDriver slpboota_driver;	/* bootleg */
extern struct GameDriver ssi_driver;		/* (c) 1990 */
//extern struct GameDriver twincobr_driver;

/* Williams games */
extern struct GameDriver robotron_driver;	/* (c) 1982 */
extern struct GameDriver robotryo_driver;	/* (c) 1982 */
extern struct GameDriver stargate_driver;	/* (c) 1981 */
extern struct GameDriver joust_driver;		/* (c) 1982 */
extern struct GameDriver joustr_driver;		/* (c) 1982 */
extern struct GameDriver joustg_driver;		/* (c) 1982 */
extern struct GameDriver joustwr_driver;	/* (c) 1982 */
extern struct GameDriver sinistar_driver;	/* (c) 1982 */
extern struct GameDriver oldsin_driver;		/* (c) 1982 */
extern struct GameDriver bubbles_driver;	/* (c) 1982 */
extern struct GameDriver bubblesr_driver;	/* (c) 1982 */
extern struct GameDriver defender_driver;	/* (c) 1980 */
extern struct GameDriver splat_driver;		/* (c) 1982 */
extern struct GameDriver blaster_driver;	/* (c) 1983 */
extern struct GameDriver colony7_driver;	/* (c) 1981 Taito */
extern struct GameDriver colony7a_driver;	/* (c) 1981 Taito */
extern struct GameDriver lottofun_driver;	/* (c) 1987 H.A.R. Management */

/* Capcom games */
/* The following is a COMPLETE list of the Capcom games up to 1997, as shown on */
/* their web site. The list is sorted by production date. Some titles are in */
/* quotes, because I couldn't find the English name (might not have been exported). */
/* The name in quotes is the title Capcom used for the html page. */
extern struct GameDriver vulgus_driver;		/* (c) 1984 */
extern struct GameDriver sonson_driver;		/* (c) 1984 */
/* Sep 1984: Higemaru */
extern struct GameDriver c1942_driver;		/* (c) 1984 */
extern struct GameDriver exedexes_driver;	/* (c) 1985 */
extern struct GameDriver savgbees_driver;	/* (c) 1985 + Memetron license */
extern struct GameDriver commando_driver;	/* (c) 1985 */
extern struct GameDriver commandj_driver;	/* (c) 1985 */
extern struct GameDriver gng_driver;		/* (c) 1985 */
extern struct GameDriver gngcross_driver;	/* (c) 1985 */
extern struct GameDriver gngjap_driver;		/* (c) 1985 */
extern struct GameDriver diamond_driver;	/* (c) 1989 KH Video (NOT A CAPCOM GAME */
											/*          but runs on GnG hardware) */
extern struct GameDriver gunsmoke_driver;	/* (c) 1985 */
extern struct GameDriver gunsmrom_driver;	/* (c) 1985 + Romstar */
extern struct GameDriver gunsmokj_driver;	/* (c) 1985 */
extern struct GameDriver sectionz_driver;	/* (c) 1985 */
extern struct GameDriver trojan_driver;		/* (c) 1986 + Romstar */
extern struct GameDriver trojanj_driver;	/* (c) 1986 */
extern struct GameDriver srumbler_driver;	/* (c) 1986 */	/* aka Rush'n Crash */
extern struct GameDriver srumblr2_driver;	/* (c) 1986 */
extern struct GameDriver lwings_driver;		/* (c) 1986 */
extern struct GameDriver lwingsjp_driver;	/* (c) 1986 */
extern struct GameDriver sidearms_driver;	/* (c) 1986 */
extern struct GameDriver sidearjp_driver;	/* (c) 1986 */
/* Feb 1987: Avenger */
/* Mar 1987: Bionic Commando */
extern struct GameDriver c1943_driver;		/* (c) 1987 */
extern struct GameDriver c1943jap_driver;	/* (c) 1987 */
extern struct GameDriver blktiger_driver;	/* (c) 1987 */
extern struct GameDriver blkdrgon_driver;	/* (c) 1987 */
/* Aug 1987: Street Fighter */
extern struct GameDriver tigeroad_driver;	/* (c) 1987 + Romstar */
extern struct GameDriver f1dream_driver;	/* (c) 1988 [+ Romstar] */
extern struct GameDriver c1943kai_driver;	/* (c) 1987 */
extern struct GameDriver lastduel_driver;	/* (c) 1988 */
/* Jul 1988: Forgotten Worlds (CPS1) */
extern struct GameDriver ghouls_driver;		/* (c) 1988 (CPS1) */
extern struct GameDriver madgear_driver;	/* (c) 1989 */	/* aka Led Storm */
extern struct GameDriver strider_driver;	/* (c) 1989 (CPS1) */
extern struct GameDriver striderj_driver;	/* (c) 1989 (CPS1) */
/* Mar 1989: Dokaben */
/* Apr 1989: Dynasty Wars (CPS1) */
extern struct GameDriver willow_driver;		/* (c) 1989 (CPS1) */
extern struct GameDriver willowj_driver;	/* (c) 1989 (CPS1) */
/* Aug 1989: Dokaben 2 */
extern struct GameDriver unsquad_driver;	/* (c) 1989 (CPS1) */
/* Oct 1989: Capcom Baseball */
/* Nov 1989: Capcom World */
extern struct GameDriver ffight_driver;		/* (c) [1989] (CPS1) */
extern struct GameDriver ffightj_driver;	/* (c) [1989] (CPS1) */
extern struct GameDriver c1941_driver;		/* (c) 1990 (CPS1) */
/* Mar 1990: "capcomq" */
/* Apr 1990: Mercs (CPS1) */
extern struct GameDriver mtwins_driver;		/* (c) 1990 (CPS1) */
extern struct GameDriver chikij_driver;		/* (c) 1990 (CPS1) */
extern struct GameDriver msword_driver;		/* (c) 1990 (CPS1) */
extern struct GameDriver cawingj_driver;	/* (c) 1990 (CPS1) */
extern struct GameDriver nemo_driver;		/* (c) 1990 (CPS1) */
/* Jan 1991: "tonosama" */
/* Mar 1991: Street Fighter II (CPS1) */
/* Apr 1991: "golf" */
/* May 1991: Ataxx */
/* Jun 1991: "sangoku" */
/* Jul 1991: Three Wonders (CPS1) */
/* Sep 1991: The King of Dragons (CPS1) */
/* Oct 1991: Block Block */
/* Nov 1991: Captain Commando (CPS1) */
extern struct GameDriver knights_driver;		/* (c) 1992 (CPS1) */
/* Apr 1992: Street Fighter II' */
/* Jul 1992: Varth (CPS1) */
/* Sep 1992: Capcom World 2 */
/* Nov 1992: Warriors of Fate (CPS1) */
/* Dec 1992: Street Fighter II Turbo (CPS1) */
/* Apr 1993: Cadillacs & Dinosaurs (CPS1) */
/* May 1993: Punisher (CPS1) */
/* Jul 1993: Slam Masters */
/* Oct 1993: Super Street Fighter II */
/* Dec 1993: Muscle Bomber Duo (CPS1) */
extern struct GameDriver pnickj_driver;		/* (c) 1994 + Compile license (CPS1) */
											/* not listed on Capcom's site? */
/* Jan 1994: Dungeons & Dragons - Tower of Doom */
/* Mar 1994: Super Street Fighter II Turbo */
/* May 1994: Alien vs Predator */
/* Jun 1994: Eco Fighters */
/* Jul 1994: Dark Stalkers */
/* Sep 1994: Ring of Destruction - Slam Masters 2 */
/* Sep 1994: Quiz and Dragons (CPS1) */
/* Oct 1994: Armored Warriors */
/* Dec 1994: X-Men - Children of the Atom */
/* Jan 1995: "tonosama2" */
/* Mar 1995: Night Warriors */
/* Apr 1995: Cyberbots */
/* Jun 1995: Street Fighter - The Movie */
/* Jun 1995: Street Fighter Alpha */
/* Oct 1995: Megaman - The Power Battle (CPS1) */
/* Nov 1995: Marvel Super Heroes */
/* Nov 1995: Battle Arena Toshiden 2 */
/* Jan 1996: 19XX */
/* Feb 1996: Dungeons & Dragons - Shadow Over Mystara */
/* Mar 1996: Street Fighter Alpha 2 */
/* Jun 1996: Super Puzzle Fighter II Turbo */
/* Jul 1996: Star Gladiator */
/* Jul 1996: Megaman 2 - The Power Fighters */
/* Aug 1996: Street Fighter Zero 2 Alpha */
/* Sep 1996: "niji" */
/* Sep 1996: X-Men vs. Street Fighter */
/* Oct 1996: War-Zard */
/* Dec 1996: Street Fighter EX */
/* Feb 1997: Street Fighter III */
/* Apr 1997: Street Fighter EX Plus */
/* the following runs on very different hardware, and is not listed by Capcom on their web site */
extern struct GameDriver capbowl_driver;	/* (c) 1988 Incredible Technologies */
extern struct GameDriver clbowl_driver;		/* (c) 1989 Incredible Technologies */
extern struct GameDriver bowlrama_driver;	/* (c) 1991 P & P Marketing */

/* Gremlin 8080 games */
extern struct GameDriver blockade_driver;	/* [1977 Gremlin] */
extern struct GameDriver comotion_driver;	/* [1977 Gremlin] */
/* Hustle (missing) */
extern struct GameDriver blasto_driver;		/* [1978 Gremlin] */

/* Gremlin/Sega "VIC dual game board" games */
/* the numbers listed are the range of ROM part numbers */
extern struct GameDriver depthch_driver;	/* 50-55 [1977 Gremlin?] */
extern struct GameDriver safari_driver;		/* 57-66 [1977 Gremlin?] */
/* 112-119 Frogs */
extern struct GameDriver sspaceat_driver;	/* 139-146 (c) */
extern struct GameDriver headon_driver;		/* 163-167/192-193 (c) Gremlin */
/* ???-??? Fortress */
/* ???-??? Gee Bee */
/* 255-270  Head On 2 / Deep Scan */
extern struct GameDriver invho2_driver;		/* 271-286 (c) 1979 */
extern struct GameDriver invinco_driver;	/* 310-318 (c) 1979 */
extern struct GameDriver invds_driver;		/* 367-382 (c) 1979 */
extern struct GameDriver tranqgun_driver;	/* 413-428 (c) 1980 */
/* 450-465  Tranquilizer Gun (different version?) */
/* ???-??? Car Hunt / Deep Scan */
/* ???-??? Digger */
extern struct GameDriver spacetrk_driver;	/* 630-645 (c) 1980 */
extern struct GameDriver carnival_driver;	/* 651-666 (c) 1980 */
extern struct GameDriver pulsar_driver;		/* 790-805 (c) 1981 */

/* Sega G-80 vector games */
extern struct GameDriver spacfury_driver;	/* no copyright notice */
extern struct GameDriver spacfurc_driver;	/* (c) 1981 */
extern struct GameDriver zektor_driver;		/* (c) 1982 */
extern struct GameDriver tacscan_driver;	/* (c) */
extern struct GameDriver elim2_driver;		/* (c) 1981 Gremlin */
extern struct GameDriver elim4_driver;
extern struct GameDriver startrek_driver;	/* (c) 1982 */

/* Sega G-80 raster games */
extern struct GameDriver astrob_driver;		/* (c) 1981 */
extern struct GameDriver astrob1_driver;	/* (c) 1981 */
extern struct GameDriver s005_driver;		/* (c) 1981 */
extern struct GameDriver monsterb_driver;	/* (c) 1982 */
extern struct GameDriver spaceod_driver;	/* (c) 1981 */

/* Sega "Zaxxon hardware" games */
extern struct GameDriver zaxxon_driver;		/* (c) 1982 */
extern struct GameDriver szaxxon_driver;	/* (c) 1982 */
extern struct GameDriver futspy_driver;
extern struct GameDriver congo_driver;		/* (c) 1983 */
extern struct GameDriver tiptop_driver;		/* (c) 1983 */

/* Sega System 8 games */
extern struct GameDriver wbdeluxe_driver;	/* (c) 1986 + Escape license */
extern struct GameDriver upndown_driver;
extern struct GameDriver wbml_driver;		/* (c) 1987 */
extern struct GameDriver pitfall2_driver;	/* reprogrammed, (c) 1984 Activision */
extern struct GameDriver chplft_driver;
extern struct GameDriver chplftb_driver;	/* (c) 1985, (c) 1982 Dan Gorlin */
extern struct GameDriver chplftbl_driver;	/* bootleg */

/* Sega System 16 games */
extern struct GameDriver passshtb_driver;	/* bootleg */
extern struct GameDriver shinobi_driver;	/* (c) 1987 */
extern struct GameDriver aliensyn_driver;	/* (c) 1987 */
extern struct GameDriver tetrisbl_driver;	/* (c) 1988 */
extern struct GameDriver altbeast_driver;	/* (c) 1988 */
extern struct GameDriver goldnaxe_driver;	/* (c) 1989 */

/* Data East "Burger Time hardware" games */
extern struct GameDriver lnc_driver;		/* (c) 1981 */
extern struct GameDriver zoar_driver;		/* (c) 1982 */
extern struct GameDriver btime_driver;		/* (c) 1982 + Midway */
extern struct GameDriver btimea_driver;		/* (c) 1982 */
extern struct GameDriver hamburge_driver;	/* bootleg */
extern struct GameDriver bnj_driver;		/* (c) 1982 + Midway */
extern struct GameDriver brubber_driver;	/* (c) 1982 */
extern struct GameDriver caractn_driver;	/* bootleg */
extern struct GameDriver eggs_driver;		/* (c) 1983 Universal USA */
extern struct GameDriver scregg_driver;		/* TA-0001 (c) 1983 Technos Japan */

/* other Data East games */
extern struct GameDriver astrof_driver;		/* (c) [1980?] */
extern struct GameDriver astrof2_driver;	/* (c) [1980?] */
extern struct GameDriver firetrap_driver;	/* (c) 1986 */
extern struct GameDriver firetpbl_driver;	/* bootleg */
extern struct GameDriver brkthru_driver;	/* (c) 1986 */
extern struct GameDriver darwin_driver;		/* (c) 1986 */
extern struct GameDriver shootout_driver;	/* (c) 1985 */
extern struct GameDriver sidepckt_driver;	/* (c) 1986 */

/* Data East 8-bit games */
//extern struct GameDriver ghostb_driver;
//extern struct GameDriver srdarwin_driver;
extern struct GameDriver cobracom_driver;	/* (c) 1988 */
//extern struct GameDriver gondo_driver;
//extern struct GameDriver oscar_driver;
//extern struct GameDriver lastmiss_driver;
//extern struct GameDriver shackled_driver;
//extern struct GameDriver mazeh_driver;

/* Data East 16-bit games */
extern struct GameDriver karnov_driver;		/* (c) 1987 */
extern struct GameDriver karnovj_driver;	/* (c) 1987 */
extern struct GameDriver chelnov_driver;	/* (c) 1988 */
extern struct GameDriver chelnovj_driver;	/* (c) 1988 */
/* the following ones all run on similar hardware */
extern struct GameDriver baddudes_driver;	/* (c) 1988 Data East USA */
extern struct GameDriver drgninja_driver;	/* (c) 1988 Data East */
extern struct GameDriver robocopp_driver;	/* (c) 1988 */
extern struct GameDriver heavyb_driver;
extern struct GameDriver heavyb2_driver;
extern struct GameDriver slyspy_driver;		/* (c) 1989 */
extern struct GameDriver hippodrm_driver;	/* (c) 1989 */
extern struct GameDriver midres_driver;		/* (c) 1989 */

/* Tehkan games */
extern struct GameDriver bombjack_driver;	/* (c) 1984 */
extern struct GameDriver starforc_driver;	/* (c) 1984 */
extern struct GameDriver megaforc_driver;	/* (c) 1985 + Videoware license */
extern struct GameDriver pbaction_driver;	/* (c) 1985 */

/* Tecmo games (Tehkan became Tecmo in 1986) */
extern struct GameDriver silkworm_driver;	/* 6217 - (c) 1988 */
extern struct GameDriver rygar_driver;		/* 6002 - (c) 1986 */
extern struct GameDriver rygarj_driver;		/* 6002 - (c) 1986 */
extern struct GameDriver gemini_driver;		/* (c) 1987 */
extern struct GameDriver wc90_driver;		/* (c) 1989 */
extern struct GameDriver wc90b_driver;		/* bootleg */
extern struct GameDriver gaiden_driver;		/* 6215 - (c) 1988 */
extern struct GameDriver shadoww_driver;	/* 6215 - (c) 1988 */

/* Konami games */
extern struct GameDriver pooyan_driver;		/* GX320 (c) 1982 Stern */
extern struct GameDriver pootan_driver;		/* bootleg */
extern struct GameDriver timeplt_driver;	/* GX393 (c) 1982 */
extern struct GameDriver spaceplt_driver;	/* bootleg */
extern struct GameDriver rocnrope_driver;	/* GX364 (c) 1983 + Kosuka */
extern struct GameDriver ropeman_driver;	/* bootleg */
extern struct GameDriver gyruss_driver;		/* GX347 (c) 1983 */
extern struct GameDriver gyrussce_driver;	/* GX347 (c) 1983 + Centuri license */
extern struct GameDriver trackfld_driver;	/* GX361 (c) 1983 */
extern struct GameDriver hyprolym_driver;	/* GX361 (c) 1983 */
extern struct GameDriver circusc_driver;	/* GX380 (c) 1984 */
extern struct GameDriver circusc2_driver;	/* GX380 (c) 1984 */
extern struct GameDriver tp84_driver;		/* GX388 (c) 1984 */
extern struct GameDriver hyperspt_driver;	/* GX330 (c) 1984 + Centuri */
extern struct GameDriver sbasketb_driver;	/* GX405 (c) 1984 */
extern struct GameDriver mikie_driver;		/* GX469 (c) 1984 */
extern struct GameDriver roadf_driver;		/* GX461 (c) 1984 */
extern struct GameDriver yiear_driver;		/* GX407 (c) 1985 */
extern struct GameDriver shaolins_driver;	/* GX477 (c) 1985 */
extern struct GameDriver pingpong_driver;	/* GX555 (c) 1985 */
extern struct GameDriver gberet_driver;		/* GX577 (c) 1985 */
extern struct GameDriver rushatck_driver;	/* GX577 (c) 1985 */
extern struct GameDriver ironhors_driver;	/* GX560 (c) 1986 */
extern struct GameDriver farwest_driver;

/* Konami "Nemesis hardware" games */
extern struct GameDriver nemesis_driver;	/* GX400 (c) 1985 */
extern struct GameDriver nemesuk_driver;	/* GX400 (c) 1985 */
extern struct GameDriver konamigt_driver;	/* GX561 (c) 1985 */
//extern struct GameDriver salamand_driver;/* GX587 not working */

/* Konami "TMNT hardware" games */
extern struct GameDriver tmnt_driver;		/* GX963 (c) 1989 */
extern struct GameDriver punkshot_driver;	/* GX907 (c) 1990 */

/* Exidy games */
extern struct GameDriver venture_driver;	/* (c) 1981 */
extern struct GameDriver venture2_driver;	/* (c) 1981 */
extern struct GameDriver mtrap_driver;		/* (c) 1981 */
extern struct GameDriver pepper2_driver;	/* (c) 1982 */
extern struct GameDriver targ_driver;		/* (c) 1980 */
extern struct GameDriver spectar_driver;	/* (c) 1980 */
extern struct GameDriver circus_driver;		/* no copyright notice [1977?] */
extern struct GameDriver robotbwl_driver;	/* no copyright notice */
extern struct GameDriver crash_driver;		/* Exidy [1979?] */
extern struct GameDriver starfire_driver;	/* Exidy */

/* Atari vector games */
extern struct GameDriver asteroid_driver;	/* no copyright notice */
extern struct GameDriver asteroi2_driver;	/* (c) 1979 */
extern struct GameDriver astdelux_driver;	/* (c) 1980 */
extern struct GameDriver bwidow_driver;		/* (c) 1982 */
extern struct GameDriver bzone_driver;		/* (c) 1980 */
extern struct GameDriver bzone2_driver;		/* (c) 1980 */
extern struct GameDriver gravitar_driver;	/* (c) 1982 */
extern struct GameDriver llander_driver;	/* no copyright notice */
extern struct GameDriver redbaron_driver;	/* (c) 1980 */
extern struct GameDriver spacduel_driver;	/* (c) 1980 */
extern struct GameDriver tempest_driver;	/* (c) 1980 */
extern struct GameDriver temptube_driver;	/* hack */
extern struct GameDriver starwars_driver;	/* (c) 1983 */
extern struct GameDriver mhavoc_driver;		/* (c) 1983 */
extern struct GameDriver mhavoc2_driver;	/* (c) 1983 */
extern struct GameDriver mhavocrv_driver;	/* hack */
extern struct GameDriver quantum_driver;	/* (c) 1982 */
extern struct GameDriver quantum2_driver;	/* (c) 1982 */

/* Atari "Centipede hardware" games */
extern struct GameDriver centiped_driver;	/* (c) 1980 */
extern struct GameDriver centipd1_driver;	/* (c) 1980 */
extern struct GameDriver milliped_driver;	/* (c) 1982 */
extern struct GameDriver warlord_driver;	/* (c) 1980 */

/* Atari "Kangaroo hardware" games */
extern struct GameDriver kangaroo_driver;	/* (c) 1982 */
extern struct GameDriver arabian_driver;	/* (c) 1983 */

/* Atari "Missile Command hardware" games */
extern struct GameDriver missile_driver;	/* (c) 1980 */
extern struct GameDriver suprmatk_driver;	/* (c) 1980 + (c) 1981 Gencomp */

/* Atary b/w games */
extern struct GameDriver sprint1_driver;	/* no copyright notice */
extern struct GameDriver sprint2_driver;	/* no copyright notice */
extern struct GameDriver sbrkout_driver;	/* no copyright notice */
extern struct GameDriver dominos_driver;	/* no copyright notice */
extern struct GameDriver nitedrvr_driver;	/* no copyright notice [1976] */
extern struct GameDriver bsktball_driver;	/* no copyright notice */
extern struct GameDriver copsnrob_driver;	/* [1976] */

/* Atari System 1 games */
extern struct GameDriver marble_driver;		/* (c) 1984 */
extern struct GameDriver marble2_driver;	/* (c) 1984 */
extern struct GameDriver marblea_driver;	/* (c) 1984 */
extern struct GameDriver peterpak_driver;	/* (c) 1984 */
extern struct GameDriver indytemp_driver;	/* (c) 1985 */
extern struct GameDriver roadrunn_driver;	/* (c) 1985 */
extern struct GameDriver roadblst_driver;	/* (c) 1986, 1987 */

/* Atari System 2 games */
extern struct GameDriver paperboy_driver;	/* (c) 1984 */
extern struct GameDriver ssprint_driver;	/* (c) 1986 */
extern struct GameDriver csprint_driver;	/* (c) 1986 */
extern struct GameDriver a720_driver;		/* (c) 1986 */
extern struct GameDriver apb_driver;		/* (c) 1987 */

/* later Atari games */
extern struct GameDriver gauntlet_driver;	/* (c) 1985 */
extern struct GameDriver gauntir1_driver;
extern struct GameDriver gauntir2_driver;
extern struct GameDriver gaunt2p_driver;
extern struct GameDriver gaunt2_driver;		/* (c) 1986 */
extern struct GameDriver atetris_driver;	/* (c) 1988 */
extern struct GameDriver atetrisa_driver;	/* (c) 1988 */
extern struct GameDriver atetrisb_driver;	/* bootleg */
extern struct GameDriver atetcktl_driver;	/* (c) 1988 */
extern struct GameDriver toobin_driver;		/* (c) 1988 */
extern struct GameDriver klax_driver;		/* (c) 1989 */
extern struct GameDriver klaxalt_driver;	/* (c) 1989 */
extern struct GameDriver blstroid_driver;	/* (c) 1987 */
extern struct GameDriver eprom_driver;		/* (c) 1989 */
extern struct GameDriver xybots_driver;		/* (c) 1987 */

/* SNK / Rock-ola games */
extern struct GameDriver satansat_driver;	/* (c) 1981 SNK */
extern struct GameDriver zarzon_driver;		/* (c) 1981 Taito, gameplay says SNK */
extern struct GameDriver vanguard_driver;	/* (c) 1981 SNK */
extern struct GameDriver fantasy_driver;	/* (c) 1981 */
/* Pioner Balloon */
extern struct GameDriver nibbler_driver;	/* (c) 1982 */
extern struct GameDriver nibblera_driver;	/* (c) 1982 */
extern struct GameDriver warpwarp_driver;	/* (c) 1981 - different hardware */
											/* the high score table says "NAMCO" */

/* Technos games */
extern struct GameDriver mystston_driver;	/* (c) 1984 */
extern struct GameDriver matmania_driver;	/* TA-0015 (c) 1985 + Taito America license */
extern struct GameDriver excthour_driver;	/* TA-0015 (c) 1985 + Taito license */
extern struct GameDriver maniach_driver;
/* TA-0018 Renegade */
extern struct GameDriver xsleena_driver;	/* TA-0019 (c) 1986 Technos Japan */
extern struct GameDriver solarwar_driver;	/* TA-0019 (c) 1986 + Memetron license */
extern struct GameDriver ddragon_driver;	/* TA-0021 (c) 1987 Taito */
extern struct GameDriver ddragonb_driver;	/* bootleg */
/* TA-0023 China Gate */
/* TA-0024 WWF Superstars */
/* TA-0025 Champ V'Ball */
extern struct GameDriver ddragon2_driver;	/* TA-0026 (c) 1988 */
/* TA-0028 Combatribes */
extern struct GameDriver blockout_driver;	/* TA-0029 (c) 1989 + California Dreams */
/* TA-0030 Double Dragon 3 */
/* TA-0031 WWF Wrestlefest */

/* Stern "Berzerk hardware" games */
extern struct GameDriver berzerk_driver;	/* (c) 1980 */
extern struct GameDriver berzerk1_driver;	/* (c) 1980 */
extern struct GameDriver frenzy_driver;		/* (c) 1982 */
extern struct GameDriver frenzy1_driver;	/* (c) 1982 */

/* GamePlan games */
extern struct GameDriver megatack_driver;	/* (c) 1980 Centuri */
extern struct GameDriver killcom_driver;	/* (c) 1980 Centuri */
extern struct GameDriver challeng_driver;	/* (c) 1981 Centuri */
extern struct GameDriver kaos_driver;		/* (c) 1981 */

/* "stratovox hardware" games */
extern struct GameDriver route16_driver;	/* (c) 1981 Leisure and Allied (bootleg) */
extern struct GameDriver stratvox_driver;	/* Taito */

/* Zaccaria games */
extern struct GameDriver monymony_driver;	/* (c) 1983 */
extern struct GameDriver jackrabt_driver;	/* (c) 1984 */

extern struct GameDriver bagman_driver;		/* (c) 1982 Stern + Valadon license */
extern struct GameDriver sbagman_driver;	/* (c) 1984 Valadon + "Stern presents" */
extern struct GameDriver spacefb_driver;	/* (c) Nintendo */
extern struct GameDriver tutankhm_driver;	/* GX350 (c) 1982 Konami */
extern struct GameDriver tutankst_driver;	/* GX350 (c) 1982 Stern */
extern struct GameDriver junofrst_driver;	/* GX310 (c) 1983 Konami */
extern struct GameDriver ccastles_driver;	/* (c) 1983 Atari */
extern struct GameDriver blueprnt_driver;	/* (c) 1982 Bally Midway */
extern struct GameDriver omegrace_driver;	/* (c) 1981 Midway */
extern struct GameDriver bankp_driver;		/* (c) 1984 Sega */
extern struct GameDriver espial_driver;		/* (c) 1983 Thunderbolt */
extern struct GameDriver espiale_driver;	/* (c) 1983 Thunderbolt */
extern struct GameDriver cloak_driver;		/* (c) 1983 Atari */
extern struct GameDriver champbas_driver;	/* (c) 1983 Sega */
//extern struct GameDriver champbb2_driver;
extern struct GameDriver exerion_driver;	/* (c) 1983 Jaleco */
extern struct GameDriver exerionb_driver;	/* (c) 1983 Jaleco */
extern struct GameDriver foodf_driver;		/* (c) 1982 Atari */
extern struct GameDriver jack_driver;		/* (c) 1982 Cinematronics */
extern struct GameDriver zzyzzyxx_driver;	/* (c) 1982 Cinematronics */
extern struct GameDriver vastar_driver;		/* (c) 1983 Sesame Japan */
extern struct GameDriver aeroboto_driver;	/* (c) 1984 Williams (aka Jaleco's Formation-Z) */
extern struct GameDriver citycon_driver;	/* (c) 1985 Jaleco */
extern struct GameDriver jedi_driver;		/* (c) 1984 Atari */
extern struct GameDriver tankbatt_driver;	/* (c) 1980 Namco */
extern struct GameDriver liberatr_driver;	/* (c) 1982 Atari */
extern struct GameDriver wiz_driver;		/* (c) 1985 Seibu Kaihatsu */
extern struct GameDriver thepit_driver;		/* (c) 1982 Centuri */
extern struct GameDriver toki_driver;		/* (c) 1990 Datsu (bootleg) */
extern struct GameDriver snowbros_driver;	/* (c) 1990 Toaplan + Romstar license */
extern struct GameDriver gundealr_driver;	/* (c) 1990 Dooyong */
extern struct GameDriver leprechn_driver;	/* (c) 1982 Tong Electronic */
extern struct GameDriver dday_driver;		/* (c) 1982 Centuri */
extern struct GameDriver hexa_driver;		/* D. R. Korea */
extern struct GameDriver redalert_driver;	/* (c) 1981 Irem (GDI game) */
extern struct GameDriver irobot_driver;
extern struct GameDriver spiders_driver;	/* (c) 1981 Sigma */



const struct GameDriver *drivers[] =
{
	/* "Pacman hardware" games */
	&pacman_driver,
	&pacplus_driver,
	&jrpacman_driver,
	&pacmod_driver,
	&namcopac_driver,
	&pacmanjp_driver,
	&hangly_driver,
	&puckman_driver,
	&piranha_driver,
	&mspacman_driver,
	&mspacatk_driver,
	&crush_driver,
	&eyes_driver,
	&ponpoko_driver,
	&maketrax_driver,
	&pengo_driver,
	&pengoa_driver,
	&penta_driver,
	/* "Galaxian hardware" games */
	&galaxian_driver,
	&galmidw_driver,
	&galnamco_driver,
	&superg_driver,
	&galapx_driver,
	&galap1_driver,
	&galap4_driver,
	&galturbo_driver,
	&pisces_driver,
	&japirem_driver,
	&uniwars_driver,
	&warofbug_driver,
	&redufo_driver,
	&pacmanbl_driver,
	&mooncrst_driver,
	&mooncrsg_driver,
	&mooncrsb_driver,
	&fantazia_driver,
	&eagle_driver,
	&moonqsr_driver,
	&checkman_driver,
	&moonal2_driver,
	&moonal2b_driver,
	/* "Scramble hardware" (and variations) games */
	&scramble_driver,
	&atlantis_driver,
	&theend_driver,
	&ckongs_driver,
	&froggers_driver,
	&amidars_driver,
	&triplep_driver,
	&scobra_driver,
	&scobrak_driver,
	&scobrab_driver,
	&losttomb_driver,
	&anteater_driver,
	&rescue_driver,
	&minefld_driver,
	&armorcar_driver,
	&tazmania_driver,
	&stratgyx_driver,
	&stratgyb_driver,
	&darkplnt_driver,
	&hustler_driver,
	&pool_driver,
	&frogger_driver,
	&frogsega_driver,
	&frogger2_driver,
	&amidar_driver,
	&amidarjp_driver,
	&amigo_driver,
	&turtles_driver,
	&turpin_driver,
	&jumpbug_driver,
	&jbugsega_driver,
	&flyboy_driver,
	&fastfred_driver,
	&jumpcoas_driver,
	/* "Crazy Climber hardware" games */
	&cclimber_driver,
	&ccjap_driver,
	&ccboot_driver,
	&ckong_driver,
	&ckonga_driver,
	&ckongjeu_driver,
	&ckongalc_driver,
	&monkeyd_driver,
	&swimmer_driver,
	&swimmera_driver,
	&guzzler_driver,
	&seicross_driver,
	&friskyt_driver,
	/* "Phoenix hardware" (and variations) games */
	&phoenix_driver,
	&phoenixt_driver,
	&phoenix3_driver,
	&pleiads_driver,
	&pleitek_driver,
	&naughtyb_driver,
	&popflame_driver,
	/* Namco games */
	&rallyx_driver,
	&nrallyx_driver,
	&locomotn_driver,
	&jungler_driver,
	&commsega_driver,
	&bosco_driver,
	&bosconm_driver,
	&galaga_driver,
	&galaganm_driver,
	&galagabl_driver,
	&gallag_driver,
	&galagab2_driver,
	&digdugnm_driver,
	&digdugat_driver,
	&xevious_driver,
	&xeviousn_driver,
	&sxevious_driver,
	&superpac_driver,
	&superpcn_driver,
	&pacnpal_driver,
	&mappy_driver,
	&mappyjp_driver,
	&digdug2_driver,
	&todruaga_driver,
	&motos_driver,
	&pacland_driver,
	&paclandn_driver,
	&paclanda_driver,
	/* Universal games */
	&cosmica_driver,
	&panic_driver,
	&panica_driver,
	&ladybug_driver,
	&snapjack_driver,
	&cavenger_driver,
	&mrdo_driver,
	&mrdot_driver,
	&mrlo_driver,
	&mrdu_driver,
	&docastle_driver,
	&docastl2_driver,
	&dounicorn_driver,
	&dowild_driver,
	&jjack_driver,
	&dorunrun_driver,
	&spiero_driver,
	&kickridr_driver,
	/* Nintendo games */
	&dkong_driver,
	&radarscp_driver,
	&dkongjp_driver,
	&dkongjr_driver,
	&dkngjrjp_driver,
	&dkjrjp_driver,
	&dkjrbl_driver,
	&dkong3_driver,
	&mario_driver,
	&masao_driver,
	&hunchy_driver,
	&popeye_driver,
	&popeyebl_driver,
	&punchout_driver,
	&spnchout_driver,
	/* Midway 8080 b/w games */
	&seawolf_driver,
	&gunfight_driver,
	&tornbase_driver,
	&zzzap_driver,
	&maze_driver,
	&boothill_driver,
	&gmissile_driver,
	&clowns_driver,
	&spcenctr_driver,
	&invaders_driver,
	&invdelux_driver,
	&invadpt2_driver,
	&earthinv_driver,
	&spaceatt_driver,
	&invrvnge_driver,
	&galxwars_driver,
	&lrescue_driver,
	&desterth_driver,
	&cosmicmo_driver,
	&spaceph_driver,
	&rollingc_driver,
	&bandido_driver,
	&astinvad_driver,
	&schaser_driver,
	&kamikaze_driver,
	&lupin3_driver,
	/* Bally Midway "Astrocade" games */
	&wow_driver,
	&robby_driver,
	&gorf_driver,
	&gorfpgm1_driver,
	&seawolf2_driver,
	&spacezap_driver,
	&ebases_driver,
	/* Bally Midway "MCR" games */
	&solarfox_driver,
	&kick_driver,
	&tron_driver,
	&twotiger_driver,
	&domino_driver,
	&shollow_driver,
	&wacko_driver,
	&kroozr_driver,
	&journey_driver,
	&tapper_driver,
	&sutapper_driver,
	&rbtapper_driver,
	&dotron_driver,
	&destderb_driver,
	&timber_driver,
	&spyhunt_driver,
	&crater_driver,
	&rampage_driver,
	&sarge_driver,
	&xenophob_driver,
	/* Irem games */
	&mpatrol_driver,
	&mpatrolw_driver,
	&mranger_driver,
	&yard_driver,
	&vsyard_driver,
	&kungfum_driver,
	&kungfub_driver,
	&travrusa_driver,
	&motorace_driver,
	&lrunner_driver,
	/* Gottlieb/Mylstar games (Gottlieb became Mylstar in 1983) */
	&reactor_driver,
	&mplanets_driver,
	&qbert_driver,
	&qbertjp_driver,
	&sqbert_driver,
	&krull_driver,
	&mach3_driver,
	&usvsthem_driver,
	&stooges_driver,
	&qbertqub_driver,
	&curvebal_driver,
	/* older Taito games */
	&crbaloon_driver,
	&crbalon2_driver,
	/* Taito "Qix hardware" games */
	&qix_driver,
	&qix2_driver,
	&sdungeon_driver,
	&zookeep_driver,
	/* Taito "Jungle King hardware" games */
	&junglek_driver,
	&jhunt_driver,
	&elevator_driver,
	&elevatob_driver,
	&wwestern_driver,
	&frontlin_driver,
	&alpine_driver,
	&spaceskr_driver,
	/* other Taito games */
	&bublbobl_driver,
	&boblbobl_driver,
	&sboblbob_driver,
	&rastan_driver,
	&rastsaga_driver,
	&rainbow_driver,
	&rainbowe_driver,
	&arkanoid_driver,
	&arknoidu_driver,
	&arkbl2_driver,
	&arkabeta_driver,
	&arkatayt_driver,
	&superqix_driver,
	&sqixbl_driver,
	&tnzs_driver,
	&tnzs2_driver,
	&slapfigh_driver,
	&slapboot_driver,
	&slpboota_driver,
	&ssi_driver,
//	&twincobr_driver,
	/* Williams games */
	&robotron_driver,
	&robotryo_driver,
	&stargate_driver,
	&joust_driver,
	&joustr_driver,
	&joustg_driver,
	&joustwr_driver,
	&sinistar_driver,
	&oldsin_driver,
	&bubbles_driver,
	&bubblesr_driver,
	&defender_driver,
	&splat_driver,
	&blaster_driver,
	&colony7_driver,
	&colony7a_driver,
	&lottofun_driver,
	/* Capcom games */
	&vulgus_driver,
	&sonson_driver,
	&c1942_driver,
	&exedexes_driver,
	&savgbees_driver,
	&commando_driver,
	&commandj_driver,
	&gng_driver,
	&gngcross_driver,
	&gngjap_driver,
	&diamond_driver,
	&gunsmoke_driver,
	&gunsmrom_driver,
	&gunsmokj_driver,
	&sectionz_driver,
	&trojan_driver,
	&trojanj_driver,
	&srumbler_driver,
	&srumblr2_driver,
	&lwings_driver,
	&lwingsjp_driver,
	&sidearms_driver,
	&sidearjp_driver,
	&c1943_driver,
	&c1943jap_driver,
	&blktiger_driver,
	&blkdrgon_driver,
	&tigeroad_driver,
	&f1dream_driver,
	&c1943kai_driver,
	&lastduel_driver,
	&ghouls_driver,
	&madgear_driver,
	&strider_driver,
	&striderj_driver,
	&willow_driver,
	&willowj_driver,
	&unsquad_driver,
	&ffight_driver,
	&ffightj_driver,
	&c1941_driver,
	&mtwins_driver,
	&chikij_driver,
	&msword_driver,
	&cawingj_driver,
	&nemo_driver,
	&knights_driver,
	&pnickj_driver,
	&capbowl_driver,
	&clbowl_driver,
	&bowlrama_driver,
	/* Gremlin 8080 games */
	&blockade_driver,
	&comotion_driver,
	&blasto_driver,
	/* Gremlin/Sega "VIC dual game board" games */
	&depthch_driver,
	&safari_driver,
	&sspaceat_driver,
	&headon_driver,
	&invho2_driver,
	&invinco_driver,
	&invds_driver,
	&tranqgun_driver,
	&spacetrk_driver,
	&carnival_driver,
	&pulsar_driver,
	/* Sega G-80 vector games */
	&spacfury_driver,
	&spacfurc_driver,
	&zektor_driver,
	&tacscan_driver,
	&elim2_driver,
	&elim4_driver,
	&startrek_driver,
	/* Sega G-80 raster games */
	&astrob_driver,
	&astrob1_driver,
	&s005_driver,
	&monsterb_driver,
	&spaceod_driver,
	/* Sega "Zaxxon hardware" games */
	&zaxxon_driver,
	&szaxxon_driver,
	&futspy_driver,
	&congo_driver,
	&tiptop_driver,
	/* Sega System 8 games */
	&wbdeluxe_driver,
	&upndown_driver,
	&wbml_driver,
	&pitfall2_driver,
	&chplft_driver,
	&chplftb_driver,
	&chplftbl_driver,
	/* Sega System 16 games */
	&shinobi_driver,
	&aliensyn_driver,
	&tetrisbl_driver,
	&passshtb_driver,
	&altbeast_driver,
	&goldnaxe_driver,
	/* Data East "Burger Time hardware" games */
	&lnc_driver,
	&zoar_driver,
	&btime_driver,
	&btimea_driver,
	&hamburge_driver,
	&bnj_driver,
	&brubber_driver,
	&caractn_driver,
	&eggs_driver,
	&scregg_driver,
	/* other Data East games */
	&astrof_driver,
	&astrof2_driver,
	&firetrap_driver,
	&firetpbl_driver,
	&brkthru_driver,
	&darwin_driver,
	&shootout_driver,
	&sidepckt_driver,
	/* Data East 8-bit games */
//	&ghostb_driver,
//	&srdarwin_driver,
	&cobracom_driver,
//	&gondo_driver,
//	&oscar_driver,
//	&lastmiss_driver,
//	&shackled_driver,
//	&mazeh_driver,
	/* Data East 16-bit games */
	&karnov_driver,
	&karnovj_driver,
	&chelnov_driver,
	&chelnovj_driver,
	&baddudes_driver,
	&drgninja_driver,
	&robocopp_driver,
	&heavyb_driver,
	&heavyb2_driver,
	&slyspy_driver,
	&hippodrm_driver,
	&midres_driver,
	/* Tehkan games */
	&bombjack_driver,
	&starforc_driver,
	&megaforc_driver,
	&pbaction_driver,
	/* Tecmo games (Tehkan became Tecmo in 1986) */
	&silkworm_driver,
	&rygar_driver,
	&rygarj_driver,
	&gemini_driver,
	&wc90_driver,
	&wc90b_driver,
	&gaiden_driver,
	&shadoww_driver,
	/* Konami games */
	&pooyan_driver,
	&pootan_driver,
	&timeplt_driver,
	&spaceplt_driver,
	&rocnrope_driver,
	&ropeman_driver,
	&gyruss_driver,
	&gyrussce_driver,
	&trackfld_driver,
	&hyprolym_driver,
	&circusc_driver,
	&circusc2_driver,
	&tp84_driver,
	&hyperspt_driver,
	&sbasketb_driver,
	&mikie_driver,
	&roadf_driver,
	&yiear_driver,
	&shaolins_driver,
	&pingpong_driver,
	&gberet_driver,
	&rushatck_driver,
	&ironhors_driver,
	&farwest_driver,
	/* Konami "Nemesis hardware" games */
	&nemesis_driver,
	&nemesuk_driver,
	&konamigt_driver,
//	&salamand_driver,
	/* Konami "TMNT hardware" games */
	&tmnt_driver,
	&punkshot_driver,
	/* Exidy games */
	&venture_driver,
	&venture2_driver,
	&mtrap_driver,
	&pepper2_driver,
	&targ_driver,
	&spectar_driver,
	&circus_driver,
	&robotbwl_driver,
	&crash_driver,
	&starfire_driver,
	/* Atari vector games */
	&asteroid_driver,
	&asteroi2_driver,
	&astdelux_driver,
	&bwidow_driver,
	&bzone_driver,
	&bzone2_driver,
	&gravitar_driver,
	&llander_driver,
	&redbaron_driver,
	&spacduel_driver,
	&tempest_driver,
	&temptube_driver,
	&starwars_driver,
	&mhavoc_driver,
	&mhavoc2_driver,
	&mhavocrv_driver,
	&quantum_driver,
	&quantum2_driver,
	/* Atari "Centipede hardware" games */
	&centiped_driver,
	&centipd1_driver,
	&milliped_driver,
	&warlord_driver,
	/* Atari "Kangaroo hardware" games */
	&kangaroo_driver,
	&arabian_driver,
	/* Atari "Missile Command hardware" games */
	&missile_driver,
	&suprmatk_driver,
	/* Atary b/w games */
	&sprint1_driver,
	&sprint2_driver,
	&sbrkout_driver,
	&dominos_driver,
	&nitedrvr_driver,
	&bsktball_driver,
	&copsnrob_driver,
	/* Atari System 1 games */
	&marble_driver,
	&marble2_driver,
	&marblea_driver,
	&peterpak_driver,
	&indytemp_driver,
	&roadrunn_driver,
	&roadblst_driver,
	/* Atari System 2 games */
	&paperboy_driver,
	&ssprint_driver,
	&csprint_driver,
	&a720_driver,
	&apb_driver,
	/* later Atari games */
	&gauntlet_driver,
	&gauntir1_driver,
	&gauntir2_driver,
	&gaunt2p_driver,
	&gaunt2_driver,
	&atetris_driver,
	&atetrisa_driver,
	&atetrisb_driver,
	&atetcktl_driver,
	&toobin_driver,
	&klax_driver,
	&klaxalt_driver,
	&blstroid_driver,
	&eprom_driver,
	&xybots_driver,
	/* SNK / Rock-ola games */
	&satansat_driver,
	&zarzon_driver,
	&vanguard_driver,
	&fantasy_driver,
	&nibbler_driver,
	&nibblera_driver,
	&warpwarp_driver,
	/* Technos games */
	&mystston_driver,
	&matmania_driver,
	&excthour_driver,
	&maniach_driver,
	&xsleena_driver,
	&solarwar_driver,
	&ddragon_driver,
	&ddragonb_driver,
	&ddragon2_driver,
	&blockout_driver,
	/* Stern "Berzerk hardware" games */
	&berzerk_driver,
	&berzerk1_driver,
	&frenzy_driver,
	&frenzy1_driver,
	/* GamePlan games */
	&megatack_driver,
	&killcom_driver,
	&challeng_driver,
	&kaos_driver,
	/* "stratovox hardware" games */
	&route16_driver,
	&stratvox_driver,

	/* Zaccaria games */
	&monymony_driver,
	&jackrabt_driver,


	&bagman_driver,
	&sbagman_driver,
	&spacefb_driver,
	&tutankhm_driver,
	&tutankst_driver,
	&junofrst_driver,
	&ccastles_driver,
	&blueprnt_driver,
	&omegrace_driver,
	&bankp_driver,
	&espial_driver,
	&espiale_driver,
	&cloak_driver,
	&champbas_driver,
//	&champbb2_driver,
	&exerion_driver,
	&exerionb_driver,
	&foodf_driver,
	&jack_driver,
	&zzyzzyxx_driver,
	&vastar_driver,
	&aeroboto_driver,
	&citycon_driver,
	&jedi_driver,
	&tankbatt_driver,
	&liberatr_driver,
	&wiz_driver,
	&thepit_driver,
	&toki_driver,
	&snowbros_driver,
	&gundealr_driver,
	&leprechn_driver,
	&dday_driver,
	&hexa_driver,
	&redalert_driver,
	&irobot_driver,
	&spiders_driver,
	0	/* end of array */
};
