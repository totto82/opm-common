/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <algorithm>

#include <fnmatch.h>
#include <opm/parser/eclipse/EclipseState/Schedule/Well/WellMatcher.hpp>

namespace Opm {

WellMatcher::WellMatcher(const std::vector<std::string>& wells) :
    m_wells(wells)
{}

WellMatcher::WellMatcher(const std::vector<std::string>& wells, const WListManager &wlm) :
    m_wells(wells),
    m_wlm(wlm)
{}

const std::vector<std::string>& WellMatcher::wells() const {
    return this->m_wells;
}

std::vector<std::string> WellMatcher::wells(const std::string& pattern) const {
    if (pattern.size() == 0)
        return {};

    // WLIST
    if (pattern[0] == '*' && pattern.size() > 1)
        return this->m_wlm.wells(pattern);

    // Normal pattern matching
    auto star_pos = pattern.find('*');
    if (star_pos != std::string::npos) {
        std::vector<std::string> names;
        for (const auto& wname : this->m_wells) {
            int flags = 0;
            if (fnmatch(pattern.c_str(), wname.c_str(), flags) == 0)
                names.push_back(wname);
        }
        return names;
    }

    auto name_iter = std::find(this->m_wells.begin(), this->m_wells.end(), pattern);
    if (name_iter != this->m_wells.end())
        return { pattern };

    return {};
}

}