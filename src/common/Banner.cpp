/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void Trinity::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
    log(Trinity::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName).c_str());
    log(R"(<Ctrl-C> to stop.)");


    log(R"(+------------------------------------------------------------------------------+)");
    log(R"(      _____ _                     _               _                   _         )");
    log(R"(     |_   _| |__  _   _ _ __   __| | ___ _ __ ___| |__   __ _ _ __ __| |        )");
    log(R"(       | | | '_ \| | | | '_ \ / _` |/ _ \ '__/ __| '_ \ / _` | '__/ _` |        )");
    log(R"(       | | | | | | |_| | | | | (_| |  __/ |  \__ \ | | | (_| | | | (_| |        )");
    log(R"(       |_| |_| |_|\__,_|_| |_|\__,_|\___|_|  |___/_| |_|\__,_|_|  \__,_|        )");
    log(R"(+------------------------------------------------------------------------------+)");
    log(R"(                       WoW / Eternity Server | 10.1.7 (51972)                   )");
    log(R"(                          https://www.thundershard.co.uk                        )");
    log(R"(+------------------------------------------------------------------------------+)");

    if (logExtraInfo)
        logExtraInfo();
}
