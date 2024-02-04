#include "engine/path.h"

#include "game/mod.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graph_traits.hpp>

#include <vector>

namespace NotAGame {

using namespace boost;

using Graph = adjacency_list<listS, vecS, directedS, no_property, property<edge_weight_t, int>>;
using Vertex = graph_traits<Graph>::vertex_descriptor;
using Edge = std::pair<Size, Size>;
using Edges = std::vector<Edge>;
using Weights = std::vector<Size>;

void AddLinks(Edges &Edges, Weights &Weights, const GlobalMap &Map, Dim X, Dim Y, Dim Z,
              const GameplaySystems &Systems, const Mod &M, const Unit &LeaderUnit) noexcept {
  const Coord3D From{X, Y, Z};
  // TODO: consider leader perks.
  const auto &Tile = Map.GetTile(Z, X, Y);

  for (const auto Neighboor :
       {Coord3D{X - 1, Y - 1, Z}, Coord3D{X, Y - 1, Z}, Coord3D{X + 1, Y - 1, Z},
        Coord3D{X - 1, Y, Z}, Coord3D{X + 1, Y, Z}, Coord3D{X - 1, Y + 1, Z}, Coord3D{X, Y + 1, Z},
        Coord3D{X + 1, Y + 1, Z}}) {
    if (!Map.IsValid(Neighboor)) {
      continue;
    }
    const auto &NeighTile = Map.GetTile(Neighboor);
    if (NeighTile.Object_.IsInvalid()) {
      Edges.push_back({Map.Coord3DToIndex(From), Map.Coord3DToIndex(Neighboor)});
      const auto Weight = Tile.Object_.IsValid()
                              ? 0
                              : M.GetTerrains().GetObjectById(NeighTile.Terrain_).GetBaseCost();
      Weights.push_back(Weight);
    }
  }
}

std::optional<Path> TryBuildPath(Coord3D From, Coord3D To, const GlobalMap &Map,
                                 const GameplaySystems &Systems, const Mod &M,
                                 const Id<Unit> LeaderId) noexcept {

  // edges of the graph
  size_t NumNodes = Map.GetWidth() * Map.GetHeight() * Map.GetNumLayers();
  // Graph is 8-bound on each plane.
  size_t GraphSize = NumNodes * 8;
  Edges Edges;
  Edges.reserve(GraphSize);
  Weights Weights;
  Weights.reserve(GraphSize);

  const auto &LeaderUnit = Systems.Units.GetComponent(LeaderId);

  for (Dim Z = 0, ZE = Map.GetNumLayers(); Z < ZE; ++Z) {
    for (Dim Y = 0, YE = Map.GetHeight(); Y < YE; ++Y) {
      for (Dim X = 0; X < Map.GetWidth(); ++X) {
        const auto &Tile = Map.GetTile(Z, X, Y);
        AddLinks(Edges, Weights, Map, X, Y, Z, Systems, M, LeaderUnit);
      }
    }
  }

  // graph created from the list of edges
  Graph G(Edges.data(), Edges.data() + Edges.size(), Weights.data(), NumNodes);

  // create vectors to store the predecessors (p) and the distances from the root (d)
  std::vector<Vertex> Pred(num_vertices(G));
  std::vector<Size> Distances(num_vertices(G));
  // create a descriptor for the source node
  Vertex Start = vertex(Map.Coord3DToIndex(From), G);

  // evaluate dijkstra on graph g with source s, predecessor_map p and distance_map d
  // note that predecessor_map(..).distance_map(..) is a bgl_named_params<P, T, R>, so a named
  // parameter
  dijkstra_shortest_paths(
      G, Start,
      predecessor_map(boost::make_iterator_property_map(Pred.begin(), get(boost::vertex_index, G)))
          .distance_map(
              boost::make_iterator_property_map(Distances.begin(), get(boost::vertex_index, G))));

  auto Coord = To;
  Path P;
  P.Start = From;
  P.Target = To;

  while (true) {
    P.Waypoints.push_back(Waypoint{.Coord = Coord});
    if (Coord == From) {
      P.Waypoints.back().Cost = 0;
      break;
    }
    const auto &Tile = Map.GetTile(Coord);
    auto TileVertexIndex = Map.Coord3DToIndex(Coord);
    P.Waypoints.back().Cost = Distances[TileVertexIndex];
    Coord = Map.IndexToCoord3D(Pred[TileVertexIndex]);
  }

  std::reverse(P.Waypoints.begin(), P.Waypoints.end());
  return std::move(P);
}

} // namespace NotAGame
