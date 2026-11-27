#include "MapRenderer.h"
#include "Creature.h"
#include "Map.h"

#include <algorithm>
#include <iostream>
#include <iterator>
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

    std::vector<const Creature*> visible;
    std::copy_if(creatures.begin(), creatures.end(), std::back_inserter(visible),
                 [&map](const Creature* creature)
                 {
                     return creature != nullptr && creature->isAlive() &&
                            map.inBounds(creature->getX(), creature->getY());
                 });

    struct Glyph
    {
        std::size_t x;
        std::size_t y;
        char symbol;
    };
    std::vector<Glyph> glyphs;
    glyphs.reserve(visible.size());
    std::transform(visible.begin(), visible.end(), std::back_inserter(glyphs),
                   [](const Creature* creature)
                   {
                       return Glyph{static_cast<std::size_t>(creature->getX()),
                                    static_cast<std::size_t>(creature->getY()),
                                    creature->getSymbol()};
                   });
    for (const Glyph& glyph : glyphs)
    {
        canvas[glyph.y][glyph.x] = glyph.symbol;
    }

    for (const std::string& row : canvas)
    {
        std::cout << row << '\n';
    }
}
