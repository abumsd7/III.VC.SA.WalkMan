#pragma once
#ifndef MUSIC_PLAYER_SA_TRACKNAMES_H
#define MUSIC_PLAYER_SA_TRACKNAMES_H

#ifdef GTASA
#include <string>
#include <unordered_map>
#include <vector>
#include <extensions/Paths.h>
#include "ConfigHelper.h"

class MusicPlayerSaTrackNames {
public:
    static inline std::unordered_map<int, std::string> trackNames;

    static inline void SetDefaults() {
        trackNames.clear();

        // ==========================================
        // 0: PLAYBACK FM (CH) - 11 Songs
        // ==========================================
        trackNames[46] = "Masta Ace - Me and the Biz";
        trackNames[53] = "Big Daddy Kane - Warm It Up, Kane";
        trackNames[60] = "Kool G Rap & DJ Polo - Road to the Riches";
        trackNames[67] = "Public Enemy - Rebel Without a Pause";
        trackNames[74] = "Rob Base and DJ E-Z Rock - It Takes Two";
        trackNames[81] = "Eric B. & Rakim - I Know You Got Soul";
        trackNames[88] = "Brand Nubian - Brand Nubian";
        trackNames[95] = "Slick Rick - Children's Story";
        trackNames[102] = "Gang Starr - B.Y.S.";
        trackNames[116] = "Biz Markie - The Vapors";
        trackNames[123] = "Spoonie Gee - The Godfather";

        // ==========================================
        // 1: K-ROSE (CO) - 15 Songs
        // ==========================================
        trackNames[231] = "Jerry Reed - Amos Moses";
        trackNames[238] = "Conway Twitty & Loretta Lynn - Louisiana Woman, Mississippi Man";
        trackNames[245] = "The Desert Rose Band - One Step Forward";
        trackNames[252] = "Statler Brothers - New York City";
        trackNames[259] = "Statler Brothers - Bed of Rose's";
        trackNames[266] = "Asleep At The Wheel - The Letter That Johnny Walker Read";
        trackNames[273] = "Juice Newton - Queen of Hearts";
        trackNames[280] = "Hank Williams - Hey Good Lookin'";
        trackNames[287] = "Patsy Cline - Three Cigarettes in an Ashtray";
        trackNames[294] = "Eddie Rabbitt - I Love a Rainy Night";
        trackNames[301] = "Willie Nelson - Crazy";
        trackNames[308] = "Mickey Gilley - Make the World Go Away";
        trackNames[135] = "Ed Bruce - Mammas Don't Let Your Babies Grow Up to Be Cowboys";
        trackNames[142] = "Merle Haggard - Always Wanting You";
        trackNames[149] = "Whitey Shafer - All My Ex's Live in Texas";

        // ==========================================
        // 2: K-DST (CR) - 15 Songs
        // ==========================================
        trackNames[365] = "Heart - Barracuda";
        trackNames[372] = "Rod Stewart - Young Turks";
        trackNames[379] = "David Bowie - Somebody Up There Likes Me";
        trackNames[386] = "Grand Funk Railroad - Some Kind of Wonderful";
        trackNames[393] = "Kiss - Strutter";
        trackNames[400] = "Toto - Hold the Line";
        trackNames[407] = "Creedence Clearwater Revival - Green River";
        trackNames[414] = "The Who - Eminence Front";
        trackNames[421] = "America - A Horse with No Name";
        trackNames[428] = "Foghat - Slow Ride";
        trackNames[435] = "Billy Idol - White Wedding Pt.1";
        trackNames[442] = "Humble Pie - Shine On";
        trackNames[449] = "Eddie Money - Two Tickets to Paradise";
        trackNames[456] = "Boston - Smokin'";
        trackNames[463] = "Lynyrd Skynyrd - Free Bird";

        // ==========================================
        // 3: BOUNCE FM (DS) - 17 Songs
        // ==========================================
        trackNames[513] = "Ohio Players - Love Rollercoaster";
        trackNames[520] = "George Clinton - Loopzilla";
        trackNames[527] = "Cameo - Candy";
        trackNames[534] = "Rick James - Cold Blooded";
        trackNames[541] = "Zapp - I Can Make You Dance";
        trackNames[547] = "Kool & the Gang - Hollywood Swinging";
        trackNames[554] = "Maze - Twilight";
        trackNames[561] = "Ronnie Hudson - West Coast Poplock";
        trackNames[564] = "Lakeside - Fantastic Voyage";
        trackNames[570] = "Dazz Band - Let It Whip";
        trackNames[577] = "The Isley Brothers - Between the Sheets";
        trackNames[584] = "MFSB - Love Is The Message";
        trackNames[591] = "Ohio Players - Funky Worm";
        trackNames[598] = "Johnny Harris - Odyssey";
        trackNames[605] = "Removed: Fatback Band - Yum Yum (Gimme Some)";
        trackNames[612] = "Removed: Roy Ayers - Running Away";
        trackNames[619] = "Removed: The Gap Band - You Dropped a Bomb on Me";

        // ==========================================
        // 4: SF-UR (HC) - 17 Songs
        // ==========================================
        trackNames[827] = "Joe Smooth - Promised Land";
        trackNames[834] = "808 State - Pacific State";
        trackNames[841] = "A Guy Called Gerald - Voodoo Ray";
        trackNames[848] = "Frankie Knuckles - Your Love";
        trackNames[855] = "Raze - Break 4 Love";
        trackNames[862] = "Cultural Vibe - Ma Foom Bey";
        trackNames[869] = "Jomanda - Make My Body Rock";
        trackNames[876] = "CeCe Rogers - Someday";
        trackNames[883] = "Nightwriters - Let The Music Use You";
        trackNames[890] = "Mr. Fingers - Can You Feel It?";
        trackNames[897] = "Marshall Jefferson - Move Your Body";
        trackNames[904] = "Maurice - This Is Acid";
        trackNames[911] = "The Todd Terry Project - Weekend";
        trackNames[918] = "Fallout - The Morning After";
        trackNames[925] = "Robert Owens - I'll Be Your Friend";
        trackNames[932] = "28th Street Crew - I Need A Rhythm";
        trackNames[939] = "Removed: Faze Action - In The Trees";

        // ==========================================
        // 5: RADIO LOS SANTOS (MH) - 16 Songs
        // ==========================================
        trackNames[981] = "Dr. Dre - Fuck wit Dre Day";
        trackNames[986] = "Dr. Dre - Nuthin' But a 'G' Thang";
        trackNames[991] = "Compton's Most Wanted - Hood Took Me Under";
        trackNames[996] = "The D.O.C. - It's Funky Enough";
        trackNames[1001] = "N.W.A - Alwayz into Somethin'";
        trackNames[1006] = "Kid Frost - La Raza";
        trackNames[1011] = "Cypress Hill - How I Could Just Kill a Man";
        trackNames[1016] = "Above the Law - Murder Rap";
        trackNames[1021] = "Eazy-E - Eazy-Er Said Than Dunn";
        trackNames[1026] = "Da Lench Mob - Guerillas in tha Mist";
        trackNames[1031] = "Ice Cube - It Was a Good Day";
        trackNames[1036] = "Ice Cube - Check Yo Self (The Message Remix)";
        trackNames[1041] = "Dr. Dre & Snoop Dogg - Deep Cover (Mixed)";
        trackNames[1046] = "Too $hort - The Ghetto";
        trackNames[1051] = "Removed: 2Pac - I Don't Give A Fuck";
        trackNames[1056] = "Removed: N.W.A - Express Yourself";

        // ==========================================
        // 6: RADIO X (MR) - 16 Songs
        // ==========================================
        trackNames[1111] = "Faith No More - Midlife Crisis";
        trackNames[1118] = "Primal Scream - Movin' On Up";
        trackNames[1125] = "Depeche Mode - Personal Jesus";
        trackNames[1132] = "Danzig - Mother";
        trackNames[1139] = "Helmet - Unsung";
        trackNames[1146] = "Living Colour - Cult of Personality";
        trackNames[1152] = "Guns N' Roses - Welcome To The Jungle";
        trackNames[1158] = "Jane's Addiction - Been Caught Stealing";
        trackNames[1164] = "Soundgarden - Rusty Cage";
        trackNames[1169] = "L7 - Pretend We're Dead";
        trackNames[1176] = "The Stone Roses - Fools Gold";
        trackNames[1183] = "Alice in Chains - Them Bones";
        trackNames[1190] = "Stone Temple Pilots - Plush";
        trackNames[1195] = "Removed? Jane's Addiction - Been Caught Stealing";
        trackNames[1202] = "Removed?";
        trackNames[1208] = "Removed? Rage Against the Machine - Killing in the Name";

        // ==========================================
        // 7: CSR 103.9 (NJ) - 15 Songs
        // ==========================================
        trackNames[1259] = "Soul II Soul - Keep On Movin'";
        trackNames[1266] = "Samuelle - So You Like What You See";
        trackNames[1273] = "Ralph Tresvant - Sensitivity";
        trackNames[1280] = "En Vogue - My Lovin' (You're Never Gonna Get It)";
        trackNames[1287] = "SWV - I'm So Into You";
        trackNames[1294] = "Guy - Groove Me";
        trackNames[1301] = "Johnny Gill - Rub You the Right Way";
        trackNames[1308] = "Boyz II Men - Motownphilly";
        trackNames[1315] = "Bobby Brown - Don't Be Cruel";
        trackNames[1322] = "Aaron Hall - Don't Be Afraid";
        trackNames[1329] = "Bell Biv DeVoe - Poison";
        trackNames[1336] = "Wreckx-n-Effect - New Jack Swing";
        trackNames[1343] = "Today - I Got the Feeling";
        trackNames[1350] = "Removed?";
        trackNames[1357] = "Removed?";

        // ==========================================
        // 8: K-JAH WEST (RE) - 13 Songs
        // ==========================================
        trackNames[1399] = "Augustus Pablo - King Tubby Meets Rockers Uptown";
        trackNames[1406] = "Toots & The Maytals - Funky Kingston";
        trackNames[1413] = "Dennis Brown - Revolution";
        trackNames[1420] = "I-Roy - Sidewalk Killer";
        trackNames[1427] = "Shabba Ranks - Wicked Inna Bed";
        trackNames[1434] = "Buju Banton - Batty Rider";
        trackNames[1441] = "Dillinger - Cocaine In My Brain";
        trackNames[1448] = "Willie Williams - Armagideon Time";
        trackNames[1455] = "Barrington Levy - Here I Come";
        trackNames[1462] = "Black Uhuru - Great Train Robbery";
        trackNames[1469] = "Reggie Stepper - Drum Pan Sound";
        trackNames[1476] = "The Maytals - Pressure Drop";
        trackNames[1483] = "Max Romeo & The Upsetters - Chase The Devil";

        // ==========================================
        // 9: MASTER SOUNDS 98.3 (RG) - 16 Songs
        // ==========================================
        trackNames[1542] = "Booker T. & the M.G.'s - Green Onions";
        trackNames[1549] = "Maceo & The Macks - Cross The Tracks (We Better Go Back)";
        trackNames[1556] = "Bobby Byrd - Hot Pants";
        trackNames[1563] = "Lyn Collins - Think (About It)";
        trackNames[1570] = "Bob James - Nautilus";
        trackNames[1577] = "The Chakachas - Jungle Fever";
        trackNames[1584] = "War - Low Rider";
        trackNames[1591] = "Gloria Jones - Tainted Love";
        trackNames[1598] = "Sir Joe Quarterman - (I Got) so Much Trouble In My Mind";
        trackNames[1605] = "Lyn Collins - Rock Me Again & Again & Again...";
        trackNames[1612] = "Bobby Byrd - I Know You Got Soul";
        trackNames[1619] = "Harlem Underground Band - Smokin' Cheeba Cheeba";
        trackNames[1625] = "Unknown Track 12";
        trackNames[1632] = "Unknown Track 13";
        trackNames[1639] = "Unknown Track 14";
        trackNames[1645] = "Unknown Track 15";

        // ==========================================
        // 10: WCTR (TK) - Talk Radio
        // ==========================================
        trackNames[1706] = "WCTR - West Coast Talk Radio 1";
        trackNames[1713] = "WCTR - West Coast Talk Radio 2";
        trackNames[1720] = "WCTR - West Coast Talk Radio 3";
        trackNames[1727] = "WCTR - West Coast Talk Radio 4";
        trackNames[1734] = "WCTR - West Coast Talk Radio 5";
        trackNames[1741] = "WCTR - West Coast Talk Radio 6";
        trackNames[1748] = "WCTR - West Coast Talk Radio 7";
        trackNames[1754] = "WCTR - West Coast Talk Radio 8";
        trackNames[1761] = "WCTR - West Coast Talk Radio 9";
        trackNames[1768] = "WCTR - West Coast Talk Radio 10";
        trackNames[1775] = "WCTR - West Coast Talk Radio 11";
        trackNames[1782] = "WCTR - West Coast Talk Radio 12";
        trackNames[1787] = "Unknown Track 12";
        trackNames[1790] = "Unknown Track 13";
        trackNames[1796] = "Unknown Track 14";
        trackNames[1803] = "Unknown Track 15";
        trackNames[1809] = "Unknown Track 16";
        trackNames[1814] = "Unknown Track 17";
    }

    static inline const std::vector<std::pair<std::string, std::vector<int>>>& GetSections() {
        static const std::vector<std::pair<std::string, std::vector<int>>> sections = {
            {"PLAYBACK FM", {46, 53, 60, 67, 74, 81, 88, 95, 102, 116, 123}},
            {"K-ROSE", {231, 238, 245, 252, 259, 266, 273, 280, 287, 294, 301, 308, 135, 142, 149}},
            {"K-DST", {365, 372, 379, 386, 393, 400, 407, 414, 421, 428, 435, 442, 449, 456, 463}},
            {"BOUNCE FM", {513, 520, 527, 534, 541, 547, 554, 561, 564, 570, 577, 584, 591, 598, 605, 612, 619}},
            {"SF-UR", {827, 834, 841, 848, 855, 862, 869, 876, 883, 890, 897, 904, 911, 918, 925, 932, 939}},
            {"RADIO LOS SANTOS", {981, 986, 991, 996, 1001, 1006, 1011, 1016, 1021, 1026, 1031, 1036, 1041, 1046, 1051, 1056}},
            {"RADIO X", {1111, 1118, 1125, 1132, 1139, 1146, 1152, 1158, 1164, 1169, 1176, 1183, 1190, 1195, 1202, 1208}},
            {"CSR 103.9", {1259, 1266, 1273, 1280, 1287, 1294, 1301, 1308, 1315, 1322, 1329, 1336, 1343, 1350, 1357}},
            {"K-JAH WEST", {1399, 1406, 1413, 1420, 1427, 1434, 1441, 1448, 1455, 1462, 1469, 1476, 1483}},
            {"MASTER SOUNDS 98.3", {1542, 1549, 1556, 1563, 1570, 1577, 1584, 1591, 1598, 1605, 1612, 1619, 1625, 1632, 1639, 1645}},
            {"WCTR", {1706, 1713, 1720, 1727, 1734, 1741, 1748, 1754, 1761, 1768, 1775, 1782, 1787, 1790, 1796, 1803, 1809, 1814}}
        };
        return sections;
    }

    static inline void Read() {
        SetDefaults();

        std::string path = GAME_PATH("\\scripts\\walkman_tracks.ini");
        ConfigHelper cfg(path);
        if (!cfg.Exists()) {
            Write();
            return;
        }

        for (const auto& sec : GetSections()) {
            for (int id : sec.second) {
                std::string key = std::to_string(id);
                std::string val = cfg.ReadString(sec.first.c_str(), key.c_str(), "");
                if (!val.empty()) {
                    trackNames[id] = val;
                }
            }
        }
    }

    static inline void Write() {
        std::string path = GAME_PATH("\\scripts\\walkman_tracks.ini");
        ConfigHelper cfg(path);
        for (const auto& sec : GetSections()) {
            for (int id : sec.second) {
                std::string key = std::to_string(id);
                cfg.WriteString(sec.first.c_str(), key.c_str(), trackNames[id]);
            }
        }
        cfg.Format("; WalkMan GTA San Andreas Track Names Configuration\n; Format: SoundID = \"Track Title\"\n");
    }

    static inline const char* GetRealTrackTitle(int soundId) {
        auto it = trackNames.find(soundId);
        if (it != trackNames.end()) {
            return it->second.c_str();
        }
        return nullptr;
    }
};

inline const char *GetRealTrackTitle(int soundId) {
    return MusicPlayerSaTrackNames::GetRealTrackTitle(soundId);
}

#endif // GTASA
#endif // MUSIC_PLAYER_SA_TRACKNAMES_H
