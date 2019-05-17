#include <oni-server/level/oni-server-level-tile-world.h>

#include <ctime>

#include <Box2D/Box2D.h>

#include <oni-core/entities/oni-entities-manager.h>
#include <oni-core/entities/oni-entities-factory.h>
#include <oni-core/level/oni-level-chunk.h>


namespace oni {
    namespace server {
        namespace level {
            TileWorld::TileWorld(oni::entities::EntityFactory &entityFactory,
                                 b2World &physicsWorld,
                                 const math::ZLayerManager &zLevel) :
                    mEntityFactory{entityFactory},
                    mPhysicsWorld{physicsWorld},
                    mZLayerManager{zLevel},
                    mTileSizeX{10}, mTileSizeY{10},
                    //mHalfTileSizeX{mTileSizeX / 2.0f},
                    //mHalfTileSizeY{mTileSizeY / 2.0f},
                    mTilesPerChunkX{11}, mTilesPerChunkY{11},
                    mChunkSizeX{static_cast<common::u16 >(mTileSizeX * mTilesPerChunkX)},
                    mChunkSizeY{static_cast<common::u16 >(mTileSizeY * mTilesPerChunkY)},
                    mHalfChunkSizeX{static_cast<common::u16 >(mChunkSizeX / 2)},
                    mHalfChunkSizeY{static_cast<common::u16>(mChunkSizeY / 2)} {
                std::srand(std::time(nullptr));

                // NOTE: A road chunk is filled with road tiles, therefore road chunk size should be,
                // dividable by road tile size.
                assert(mTileSizeX <= mChunkSizeX);
                assert(mTileSizeY <= mChunkSizeY);
                assert(mChunkSizeX % mTileSizeX == 0);
                assert(mChunkSizeY % mTileSizeY == 0);

                // NOTE: If number of road tiles in a chunk is not an odd number it means there is no middle
                // tile. For convenience its good to have a middle tile.
                assert((mChunkSizeX / mTileSizeX) % 2 == 1);
                assert((mChunkSizeY / mTileSizeY) % 2 == 1);

                assert(mChunkSizeX % 2 == 0);
                assert(mChunkSizeY % 2 == 0);

                mNorthToEast = "resources/images/road/1/north-to-east.png";
                mNorthToSouth = "resources/images/road/1/north-to-south.png";
                mSouthToEast = "resources/images/road/1/south-to-east.png";
                mSouthToNorth = "resources/images/road/1/south-to-north.png";
                mWestToEast = "resources/images/road/1/west-to-east.png";
                mWestToNorth = "resources/images/road/1/west-to-north.png";
                mWestToSouth = "resources/images/road/1/west-to-south.png";

                mRaceTrack1 = "resources/images/race-track/2/1.png";
                mRaceTrack2 = "resources/images/race-track/2/2.png";
                mRaceTrack3 = "resources/images/race-track/2/3.png";
                mRaceTrack4 = "resources/images/race-track/2/4.png";

                mGroundZ = mZLayerManager.getZForEntity(oni::entities::EntityType::BACKGROUND);
                mRoadZ = mZLayerManager.getZForEntity(oni::entities::EntityType::ROAD);
                mWallZ = mZLayerManager.getZForEntity(oni::entities::EntityType::WALL);
            }

            TileWorld::~TileWorld() = default;


            bool
            TileWorld::isInMap(common::u64 packedIndex,
                               const std::map<common::u64, common::EntityID> &map) const {
                return map.find(packedIndex) != map.end();
            }

            math::vec2
            TileWorld::unpackCoordinates(common::u64 coord) const {
                // TODO: This function is incorrect. Need to match it to packInt64 function if I ever use it
                assert(false);
                //auto x = static_cast<int>(coord >> 32) * mTileSizeX;
                //auto y = static_cast<int>(coord & (0xFFFFFFFF)) * mTileSizeX;

                //return math::vec2{x, y};
                return math::vec2{};
            }

            void
            TileWorld::tick(const component::WorldP2D &position) {
                tickChunk(position);
            }

            void
            TileWorld::tickChunk(const component::WorldP2D &position) {
                auto chunkIndex = groundChunkPosToIndex(position);

                // NOTE: We always create and fill chunks in the current location and 8 adjacent chunks.
                // 1--2--3
                // |--|--|
                // 4--c--5
                // |--|--|
                // 6--7--8
                for (auto i = chunkIndex.x - 1; i <= chunkIndex.x + 1; ++i) {
                    for (auto j = chunkIndex.y - 1; j <= chunkIndex.y + 1; ++j) {
                        auto chunkID = math::packInt64(i, j);
                        if (!isInMap(chunkID, mChunkLookup)) {
                            // genChunkTexture(i, j);
                            // genChunkTiles(i, j);
                            // genChunkRoads(i, j);
                            genChunkGroundSprite(i, j);
                        }
                    }
                }
            }

            void
            TileWorld::genChunkRoads(common::i64 chunkX,
                                     common::i64 chunkY) {
                /**
                 * 1. Check if there should be a road in this chunk
                 * 2. Find the neighbours connected by road to current chunk
                 * 3. Find if neighbours are initialized, if yes find the tile position on the boarder of the chunk
                 *    that has a road choose the tile next to it in this chunk as the starting chunk, if neighbour is
                 *    uninitialized pick a random tile. Repeat the same for the other chunk but this time assign an end
                 *    tile.
                 * 4. Connect starting tile to the ending tile.
                 */

                oni::level::ChunkIndex chunkIndex{chunkX, chunkY};
                oni::level::EdgeRoadTile edgeRoads{};

                if (!shouldGenerateRoad(chunkIndex)) {
                    return;
                }

                auto northChunkIndex = oni::level::ChunkIndex{chunkIndex.x, chunkIndex.y + 1};
                auto northChunkID = math::packInt64(chunkIndex.x, chunkIndex.y);

                auto southChunkIndex = oni::level::ChunkIndex{chunkIndex.x, chunkIndex.y - 1};
                auto southChunkID = math::packInt64(southChunkIndex.x, southChunkIndex.y);

                auto westChunkIndex = oni::level::ChunkIndex{chunkIndex.x - 1, chunkIndex.y};
                auto westChunkID = math::packInt64(westChunkIndex.x, westChunkIndex.y);

                auto eastChunkIndex = oni::level::ChunkIndex{chunkIndex.x + 1, chunkIndex.y};
                auto eastChunkID = math::packInt64(eastChunkIndex.x, eastChunkIndex.y);

                auto northChunkHasRoads = shouldGenerateRoad(northChunkIndex);
                auto southChunkHasRoads = shouldGenerateRoad(southChunkIndex);
                auto westChunkHasRoads = shouldGenerateRoad(westChunkIndex);
                auto eastChunkHasRoads = shouldGenerateRoad(eastChunkIndex);

                auto neighboursRoadStatus = {northChunkHasRoads, southChunkHasRoads, westChunkHasRoads,
                                             eastChunkHasRoads};
                auto neighboursWithRoad = std::count_if(neighboursRoadStatus.begin(), neighboursRoadStatus.end(),
                                                        [](bool status) { return status; });
                assert(neighboursWithRoad == 2);

                oni::level::RoadTileIndex startingRoadTileIndex{0, 0};

                oni::level::RoadTileIndex endingRoadTileIndex{0, 0};

                oni::level::RoadTileIndex northBoarderRoadTileIndex{
                        static_cast<uint16>(std::rand() % mTilesPerChunkX),
                        static_cast<uint16>(mTilesPerChunkY - 1)};

                oni::level::RoadTileIndex southBoarderRoadTileIndex{
                        static_cast<uint16>(std::rand() % mTilesPerChunkX),
                        0};

                oni::level::RoadTileIndex westBoarderRoadTileIndex{0,
                                                                   static_cast<uint16>(std::rand() %
                                                                                       mTilesPerChunkY)};

                oni::level::RoadTileIndex eastBoarderRoadTileIndex{static_cast<uint16>(mTilesPerChunkX - 1),
                                                                   static_cast<uint16>(std::rand() %
                                                                                       mTilesPerChunkY)};

                if (northChunkHasRoads && southChunkHasRoads) {
                    edgeRoads.southBoarder = oni::level::RoadTileIndex{};
                    edgeRoads.northBoarder = oni::level::RoadTileIndex{};

                    if (isInMap(southChunkID, mChunkLookup)) {
                        auto southChunkEntityID = mChunkLookup.at(southChunkID);
                        const auto &southChunk = mEntityFactory.getEntityManager().get<oni::level::Chunk>(
                                southChunkEntityID);

                        edgeRoads.southBoarder.x = southChunk.edgeRoad.northBoarder.x;
                        edgeRoads.southBoarder.y = 0;
                    } else {
                        edgeRoads.southBoarder = southBoarderRoadTileIndex;
                    }
                    if (isInMap(northChunkID, mChunkLookup)) {
                        auto northChunkEntityID = mChunkLookup.at(northChunkID);
                        const auto &northChunk = mEntityFactory.getEntityManager().get<oni::level::Chunk>(
                                northChunkEntityID);

                        edgeRoads.northBoarder = northChunk.edgeRoad.southBoarder;

                    } else {
                        edgeRoads.northBoarder = northBoarderRoadTileIndex;
                    }

                    startingRoadTileIndex = edgeRoads.southBoarder;
                    endingRoadTileIndex = edgeRoads.northBoarder;

                } else if (northChunkHasRoads && eastChunkHasRoads) {

                } else if (northChunkHasRoads && westChunkHasRoads) {

                } else if (southChunkHasRoads && westChunkHasRoads) {

                } else if (southChunkHasRoads && eastChunkHasRoads) {

                } else if (westChunkHasRoads && eastChunkHasRoads) {
                    edgeRoads.westBoarder = oni::level::RoadTileIndex{};
                    edgeRoads.eastBoarder = oni::level::RoadTileIndex{};

                    if (isInMap(eastChunkID, mChunkLookup)) {
                        auto eastChunkEntityID = mChunkLookup.at(eastChunkID);
                        const auto &eastChunk = mEntityFactory.getEntityManager().get<oni::level::Chunk>(
                                eastChunkEntityID);

                        edgeRoads.eastBoarder.x = mTilesPerChunkX - 1;
                        edgeRoads.eastBoarder.y = eastChunk.edgeRoad.westBoarder.y;

                    } else {
                        edgeRoads.eastBoarder = eastBoarderRoadTileIndex;
                    }

                    if (isInMap(westChunkID, mChunkLookup)) {
                        auto westChunkEntityID = mChunkLookup.at(westChunkID);
                        const auto &westChunk = mEntityFactory.getEntityManager().get<oni::level::Chunk>(
                                westChunkEntityID);

                        edgeRoads.westBoarder.x = 0;
                        edgeRoads.westBoarder.y = westChunk.edgeRoad.eastBoarder.y;
                    } else {
                        edgeRoads.westBoarder = westBoarderRoadTileIndex;
                    }

                    startingRoadTileIndex = edgeRoads.westBoarder;
                    endingRoadTileIndex = edgeRoads.eastBoarder;

                    common::u16 currentTileX = startingRoadTileIndex.x;
                    common::u16 currentTileY = startingRoadTileIndex.y;

                    auto previousRoadTexture = mWestToEast;

                    while (currentTileX < (endingRoadTileIndex.x + 1)) {
                        if (currentTileX == endingRoadTileIndex.x) {
                            // Make sure we connect to endingRoadTile
                            if (currentTileY == endingRoadTileIndex.y) {
                                if (previousRoadTexture == mWestToEast) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mWestToEast);
                                } else if (previousRoadTexture == mSouthToNorth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mSouthToEast);
                                } else if (previousRoadTexture == mNorthToSouth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mNorthToEast);
                                } else if (previousRoadTexture == mWestToSouth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mNorthToEast);
                                } else if (previousRoadTexture == mWestToNorth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mSouthToEast);
                                } else {
                                    assert(false);
                                }
                                break;
                                // We are done
                            } else if (currentTileY > endingRoadTileIndex.y) {
                                if (previousRoadTexture == mWestToEast) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mWestToSouth);
                                    previousRoadTexture = mWestToSouth;
                                } else if (previousRoadTexture == mNorthToSouth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mNorthToSouth);
                                    previousRoadTexture = mNorthToSouth;
                                } else if (previousRoadTexture == mWestToSouth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mNorthToSouth);
                                    previousRoadTexture = mNorthToSouth;
                                } else {
                                    assert(false);
                                }
                                --currentTileY;
                                // go down
                            } else {
                                if (previousRoadTexture == mWestToEast) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mWestToNorth);
                                    previousRoadTexture = mWestToNorth;
                                } else if (previousRoadTexture == mSouthToNorth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mSouthToNorth);
                                    previousRoadTexture = mSouthToNorth;
                                } else if (previousRoadTexture == mWestToNorth) {
                                    genTileRoad(chunkIndex,
                                                oni::level::RoadTileIndex{currentTileX, currentTileY},
                                                mSouthToNorth);
                                    previousRoadTexture = mSouthToNorth;
                                } else {
                                    assert(false);
                                }
                                ++currentTileY;
                                // go up
                            }
                        } else {
                            // TODO: Randomly gen road instead of straight line
                            genTileRoad(chunkIndex,
                                        oni::level::RoadTileIndex{currentTileX, currentTileY},
                                        mWestToEast);
                            ++currentTileX;
                        }
                    }

                } else {
                    assert(false);
                }

/*            genTileRoad(chunkIndex, startingRoadTileIndex, endingRoadTileIndex,
                                    entities);*/


                auto chunkID = math::packInt64(chunkIndex.x, chunkIndex.y);
                auto worldPos = groundChunkIndexToPos(chunkIndex);
                auto chunkEntityID = mChunkLookup[chunkID];
                auto &chunk = mEntityFactory.getEntityManager().get<oni::level::Chunk>(chunkEntityID);
                auto &chunkPos = mEntityFactory.getEntityManager().get<component::WorldP3D>(chunkEntityID);
                chunkPos = worldPos;
                chunk.index = chunkID;
                chunk.edgeRoad = edgeRoads;
            }

            void
            TileWorld::genTileRoad(const oni::level::ChunkIndex &chunkIndex,
                                   const oni::level::RoadTileIndex &roadTileIndex,
                                   const std::string &texturePath) {
                auto worldPos = roadTileIndexToPos(chunkIndex, roadTileIndex);
                auto roadEntityID = genTexture(getTileSize(), worldPos, texturePath);
                auto roadID = math::packInt64(roadTileIndex.x, roadTileIndex.y);
                mRoadLookup.emplace(roadID, roadEntityID);
            }

            void
            TileWorld::genTileRoad(const oni::level::ChunkIndex &chunkIndex,
                                   const oni::level::RoadTileIndex &roadTileIndex) {
                math::vec4 color{0.1f, 0.1f, 0.1f, 0.5f};
                auto roadTileSize = getTileSize();

                auto worldPos = roadTileIndexToPos(chunkIndex, roadTileIndex);
                auto roadID = genSprite(color, roadTileSize, worldPos);

                auto packedIndex = math::packInt64(roadTileIndex.x, roadTileIndex.y);
                mRoadLookup.emplace(packedIndex, roadID);
            }

            void
            TileWorld::genTileRoad(const oni::level::ChunkIndex &chunkIndex,
                                   oni::level::RoadTileIndex startingRoadTileIndex,
                                   oni::level::RoadTileIndex endingRoadTileIndex) {
                // Fill between tiles as if we are sweeping the Manhattan distance between them.
                while (startingRoadTileIndex.x < endingRoadTileIndex.x) {
                    genTileRoad(chunkIndex, startingRoadTileIndex);
                    ++startingRoadTileIndex.x;
                }

                while (startingRoadTileIndex.x > endingRoadTileIndex.x) {
                    genTileRoad(chunkIndex, startingRoadTileIndex);
                    --startingRoadTileIndex.x;
                }

/*            if (startTilePosX == endTilePosX) {
                if (startTilePosY > endTilePosY) {
                    startTilePosY -= mTileSizeY;
                }
                if (startTilePosY < endTilePosY) {
                    startTilePosY += mTileSizeY;
                }
            }
            startTilePosX -= mTileSizeX;*/

                while (startingRoadTileIndex.y < endingRoadTileIndex.y) {
                    genTileRoad(chunkIndex, startingRoadTileIndex);
                    ++startingRoadTileIndex.y;
                }

                while (startingRoadTileIndex.y > endingRoadTileIndex.y) {
                    genTileRoad(chunkIndex, startingRoadTileIndex);
                    --startingRoadTileIndex.y;
                }
            }

            void
            TileWorld::genChunkTiles(common::i64 xChunkIndex,
                                     common::i64 yChunkIndex) {

                auto firstTileX = xChunkIndex * mChunkSizeX;
                auto lastTileX = xChunkIndex * mChunkSizeX + mChunkSizeX;
                auto firstTileY = yChunkIndex * mChunkSizeY;
                auto lastTileY = yChunkIndex * mChunkSizeY + mChunkSizeY;

                auto tileSize = getTileSize();

                for (auto i = firstTileX; i < lastTileX; i += mTileSizeX) {
                    for (auto j = firstTileY; j < lastTileY; j += mTileSizeY) {
                        auto packedIndex = math::packInt64(i, j);
                        // Chunks are created in batch, if single tile is created so are others.
                        if (isInMap(packedIndex, mTileLookup)) {
                            return;
                        }
                        auto R = (std::rand() % 255) / 255.0f;
                        auto G = (std::rand() % 255) / 255.0f;
                        auto B = (std::rand() % 255) / 255.0f;
                        auto color = math::vec4{R, G, B, 1.0f};

                        auto worldPos = component::WorldP3D{static_cast<common::r32>(i),
                                                            static_cast<common::r32>(j),
                                                            mGroundZ};

                        auto tileID = genSprite(color, tileSize, worldPos);

                        mTileLookup.emplace(packedIndex, tileID);
                    }
                }
            }

            bool
            TileWorld::shouldGenerateRoad(const oni::level::ChunkIndex &chunkIndex) const {
                return chunkIndex.y == 0;
            }

            component::WorldP3D
            TileWorld::groundChunkIndexToPos(const oni::level::ChunkIndex &chunkIndex) const {
                auto pos = component::WorldP3D{
                        static_cast<common::r32>(chunkIndex.x * mChunkSizeX),
                        static_cast<common::r32>(chunkIndex.y * mChunkSizeY),
                        // TODO: Should I keep Z as part of ChunkIndex maybe?
                        mGroundZ};
                return pos;
            }

            oni::level::ChunkIndex
            TileWorld::groundChunkPosToIndex(const component::WorldP2D &position) const {
                auto x = floor(position.x / mChunkSizeX);
                auto xIndex = static_cast<common::i64>(x);
                auto y = floor(position.y / mChunkSizeY);
                auto yIndex = static_cast<common::i64>(y);
                return oni::level::ChunkIndex{xIndex, yIndex};
            }

            component::WorldP3D
            TileWorld::roadTileIndexToPos(const oni::level::ChunkIndex &chunkIndex,
                                          oni::level::RoadTileIndex roadTileIndex) const {

                auto chunkPos = groundChunkIndexToPos(chunkIndex);
                auto tilePos = math::vec2{static_cast<common::r32>(roadTileIndex.x * mTileSizeX),
                                          static_cast<common::r32>(roadTileIndex.y * mTileSizeY)};
                auto pos = component::WorldP3D{chunkPos.x + tilePos.x, chunkPos.y + tilePos.y,
                                               chunkPos.z};
                return pos;
            }

            void
            TileWorld::createWall(const std::vector<oni::level::WallTilePosition> &position,
                                  const std::vector<oni::level::TileIndex> &indices) {
                assert(position.size() == indices.size());

                size_t wallCount = indices.size();

                std::vector<common::EntityID> wallEntities{};
                wallEntities.reserve(wallCount);

                common::r32 wallWidth = 0.5f;
                auto heading = component::Heading{0.f}; // For static objects facing angle does not matter.

                for (size_t i = 0; i < wallCount; ++i) {
                    auto &wallPos = position[i];
                    auto &xTileIndex = indices[i].x;
                    auto &yTileIndex = indices[i].y;

                    auto wallPositionInWorld = component::WorldP3D{};
                    math::vec2 wallSize;
                    std::string wallTexturePath;

                    common::r32 currentTileX = xTileIndex * mTileSizeX;
                    common::r32 currentTileY = yTileIndex * mTileSizeY;

                    switch (wallPos) {
                        case oni::level::WallTilePosition::RIGHT: {
                            wallSize.x = wallWidth;
                            wallSize.y = mTileSizeY - 2 * wallWidth;
                            wallTexturePath = "resources/images/wall/1/vertical.png";

                            wallPositionInWorld.x = currentTileX + mTileSizeX - wallWidth;
                            wallPositionInWorld.y = currentTileY + wallWidth;
                            wallPositionInWorld.z = mWallZ;
                            break;
                        }
                        case oni::level::WallTilePosition::TOP: {
                            wallSize.x = mTileSizeX - 2 * wallWidth;
                            wallSize.y = wallWidth;
                            wallTexturePath = "resources/images/wall/1/horizontal.png";

                            wallPositionInWorld.x = currentTileX + wallWidth;
                            wallPositionInWorld.y = currentTileY + mTileSizeY - wallWidth;
                            wallPositionInWorld.z = mWallZ;
                            break;
                        }
                        case oni::level::WallTilePosition::LEFT: {
                            wallSize.x = wallWidth;
                            wallSize.y = mTileSizeY - 2 * wallWidth;
                            wallTexturePath = "resources/images/wall/1/vertical.png";

                            wallPositionInWorld.x = currentTileX;
                            wallPositionInWorld.y = currentTileY + wallWidth;
                            wallPositionInWorld.z = mWallZ;
                            break;
                        }
                        case oni::level::WallTilePosition::BOTTOM: {
                            wallSize.x = mTileSizeX - 2 * wallWidth;
                            wallSize.y = wallWidth;
                            wallTexturePath = "resources/images/wall/1/horizontal.png";

                            wallPositionInWorld.x = currentTileX + wallWidth;
                            wallPositionInWorld.y = currentTileY;
                            wallPositionInWorld.z = mWallZ;
                            break;
                        }
                    }

                    auto entityID = mEntityFactory.createEntity<oni::entities::EntityType::WALL>(
                            entities::SimMode::SERVER,
                            wallPositionInWorld,
                            wallSize,
                            heading,
                            wallTexturePath);
                    mEntityFactory.tagForNetworkSync(entityID);
                }
            }

            void
            TileWorld::genChunkGroundTexture(common::i64 chunkX,
                                             common::i64 chunkY) {
                auto chunkIndex = oni::level::ChunkIndex{chunkX, chunkY};
                auto worldPos = groundChunkIndexToPos(chunkIndex);

                auto chunkEntityID = genTexture(getChunkSize(), worldPos, mRaceTrack1);
                auto packed = math::packInt64(chunkX, chunkY);
                mChunkLookup.emplace(packed, chunkEntityID);
            }

            void
            TileWorld::genChunkGroundSprite(common::i64 chunkX,
                                            common::i64 chunkY) {
                auto chunkID = math::packInt64(chunkX, chunkY);
                auto R = (std::rand() % 255) / 255.0f;
                auto G = (std::rand() % 255) / 255.0f;
                auto B = (std::rand() % 255) / 255.0f;
                math::vec4 color{R, G, B, 0.1f};
                math::vec2 size{static_cast<common::r32>(mChunkSizeX),
                                static_cast<common::r32 >(mChunkSizeY)};
                oni::level::ChunkIndex currentChunkIndex{chunkX, chunkY};
                auto worldPos = groundChunkIndexToPos(currentChunkIndex);
                auto chunkEntityID = genSprite(color, size, worldPos);

                mChunkLookup.emplace(chunkID, chunkEntityID);

            }

            math::vec2
            TileWorld::getTileSize() const {
                return math::vec2{static_cast<common::r32>(mTileSizeX),
                                  static_cast<common::r32 >(mTileSizeY)};
            }

            math::vec2
            TileWorld::getChunkSize() const {
                return math::vec2{static_cast<common::r32>(mChunkSizeX),
                                  static_cast<common::r32 >(mChunkSizeY)};
            }

            common::EntityID
            TileWorld::genSprite(math::vec4 &color,
                                 math::vec2 &tileSize,
                                 component::WorldP3D &worldPos) {
                auto heading = component::Heading{0.f};
                auto entityID = mEntityFactory.createEntity<oni::entities::EntityType::WORLD_CHUNK>(
                        entities::SimMode::SERVER,
                        worldPos, tileSize,
                        heading,
                        color);
                mEntityFactory.tagForNetworkSync(entityID);
                return entityID;
            }

            common::EntityID
            TileWorld::genTexture(const math::vec2 &size,
                                  const component::WorldP3D &worldPos,
                                  const std::string &path) {
                auto heading = component::Heading{0.f};
                auto entityID = mEntityFactory.createEntity<oni::entities::EntityType::WORLD_CHUNK>(
                        entities::SimMode::SERVER,
                        worldPos, size,
                        heading,
                        path);
                mEntityFactory.tagForNetworkSync(entityID);
                return entityID;
            }

            void
            TileWorld::genDemoRaceCourse() {
                return;
                for (int i = -2; i <= 2; ++i) {
                    for (int j = -2; j <= 2; ++j) {
                        genChunkGroundTexture(i, j);
                    }
                }

                std::vector<oni::level::WallTilePosition> wallPosInTile{};
                std::vector<oni::level::TileIndex> wallTiles{};

                common::i8 outerTrackWidth = 8;
                common::i8 outerTrackHeight = 4;

                for (auto i = -outerTrackWidth; i <= outerTrackWidth; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::BOTTOM);
                    wallTiles.emplace_back(oni::level::TileIndex{i, -outerTrackHeight});
                }

                for (auto i = -outerTrackHeight; i <= outerTrackHeight; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::RIGHT);
                    wallTiles.emplace_back(oni::level::TileIndex{outerTrackWidth, i});
                }

                for (auto i = -outerTrackWidth; i <= outerTrackWidth; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::TOP);
                    wallTiles.emplace_back(oni::level::TileIndex{i, outerTrackHeight});
                }

                for (auto i = -outerTrackHeight; i <= outerTrackHeight; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::LEFT);
                    wallTiles.emplace_back(oni::level::TileIndex{-outerTrackWidth, i});
                }

                common::i8 innerTrackWidth = 6;
                common::i8 innerTrackHeight = 2;

                for (auto i = -innerTrackWidth; i <= innerTrackWidth; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::BOTTOM);
                    wallTiles.emplace_back(oni::level::TileIndex{i, -innerTrackHeight});
                }

                for (auto i = -innerTrackHeight; i <= innerTrackHeight; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::RIGHT);
                    wallTiles.emplace_back(oni::level::TileIndex{innerTrackWidth, i});
                }

                for (auto i = -innerTrackWidth; i <= innerTrackWidth; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::TOP);
                    wallTiles.emplace_back(oni::level::TileIndex{i, innerTrackHeight});
                }

                for (auto i = -innerTrackHeight; i <= innerTrackHeight; ++i) {
                    wallPosInTile.emplace_back(oni::level::WallTilePosition::LEFT);
                    wallTiles.emplace_back(oni::level::TileIndex{-innerTrackWidth, i});
                }

                // createWall(wallPosInTile, wallTiles);
            }
        }
    }
}