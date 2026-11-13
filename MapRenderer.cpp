#include "MapRenderer.h"
#include "Creature.h"
#include "Map.h"

#include <iostream>
#include <string>

void MapRenderer::draw(const Map& map, const std::vector<const Creature*>& creatures) const
{
    std::vector<std::string> canvas(
        static_cast<std::size_t>(map.getRows()),
        std::string(static_cast<std::size_t>(map.getCols()), Map::FLOOR));

    for (int r = 0; r < map.getRows(); ++r)
    {
        for (int c = 0; c < map.getCols(); ++c)
        {
            canvas[static_cast<std::size_t>(r)][static_cast<std::size_t>(c)] = map.getCell(c, r);
        }
    }

    for (const Creature* creature : creatures)
    {
        if (creature == nullptr || !creature->isAlive())
        {
            continue;
        }
        if (map.inBounds(creature->getX(), creature->getY()))
        {
            canvas[static_cast<std::size_t>(creature->getY())]
                  [static_cast<std::size_t>(creature->getX())] = creature->getSymbol();
        }
    }

    for (const std::string& row : canvas)
    {
        std::cout << row << '\n';
    }
}
