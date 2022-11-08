#ifndef FRONTIER_SEARCH_H_
#define FRONTIER_SEARCH_H_

#include <costmap_2d/costmap_2d.h>
#include <visualization_msgs/MarkerArray.h>

namespace frontier_exploration
{

 struct punto{
  geometry_msgs::Point puntos;
  double valor;  
};

 
/**
 * @brief Represents a frontier
 *
 */
struct Frontier {
  std::uint32_t size;
  double min_distance;
  double cost;
  double entropy_gain, f_entropy;
  int known_cells, unknown_cells;
  geometry_msgs::Point initial;
  geometry_msgs::Point centroid;
  geometry_msgs::Point middle;
  std::vector<geometry_msgs::Point> points;
  geometry_msgs::Point goal;
  std::vector<punto> puntos;
  };

// struct punto{
//   geometry_msgs::Point puntos;
//   double valor;  
// };

/**
 * @brief Thread-safe implementation of a frontier-search task for an input
 * costmap.
 */
class FrontierSearch
{
friend class Explore;

public:
  FrontierSearch()
  {
  }

  /**
   * @brief Constructor for search task
   * @param costmap Reference to costmap data to search.
   */
  FrontierSearch(costmap_2d::Costmap2D* costmap, double potential_scale,
                 double gain_scale, double min_frontier_size, double expected_radius, 
                 double gain_entropy, double obstacle_gain);

  /**
   * @brief Runs search implementation, outward from the start position
   * @param position Initial position to search from
   * @return List of frontiers, if any
   */
  std::vector<Frontier> searchFrom(geometry_msgs::Point position);

  double g_unknown, fa_entropy;

  std::vector<punto> puntos; // Puntos totales de las fronteras
  std::vector<geometry_msgs::Point> points; // Vector solo de puntos

protected:
  /**
   * @brief Starting from an initial cell, build a frontier from valid adjacent
   * cells
   * @param initial_cell Index of cell to start frontier building
   * @param reference Reference index to calculate position from
   * @param frontier_flag Flag vector indicating which cells are already marked
   * as frontiers
   * @return new frontier
   */
  Frontier buildNewFrontier(unsigned int initial_cell, unsigned int reference,
                            std::vector<bool>& frontier_flag);

  /**
   * @brief isNewFrontierCell Evaluate if candidate cell is a valid candidate
   * for a new frontier.
   * @param idx Index of candidate cell
   * @param frontier_flag Flag vector indicating which cells are already marked
   * as frontiers
   * @return true if the cell is frontier cell
   */
  bool isNewFrontierCell(unsigned int idx,
                         const std::vector<bool>& frontier_flag);

  /**
   * @brief computes frontier entropy gain
   *
   * @param frontier frontier for which compute the entropy gain
   * @return entropy gain of the frontier
  */
  double frontierEntropyGain(Frontier& frontier);                       

  /**
   * @brief computes frontier cost
   * @details cost function is defined by potential_scale and gain_scale
   *
   * @param frontier frontier for which compute the cost
   * @return cost of the frontier
   */
  double frontierCost(const Frontier& frontier);

  /**
   * @brief computes current entropy
   * @details current entropy is defined here
   *
   * @param frontier frontier for which compute the current entropy
   * @return current entropy of the frontier
  */
  double currentEntropy(void);

  /**
   * @brief Return expected uknown cells of the frontier frontier
   *
   * @param frontier frontier for which compute expected uknown cells
   * @return expected uknown cells of the frontier
  */ 
  int f_expected_ucells(Frontier& frontier, int &expected_kcells, double &factor);


private:
  costmap_2d::Costmap2D* costmap_;
  unsigned char* map_;
  unsigned int size_x_, size_y_;
  double potential_scale_, gain_scale_, gain_entropy_, obstacle_gain_;
  double min_frontier_size_;
  double expected_radius_;
  std::queue<unsigned int> gknown_cells_;
  double g_unknown_, fa_entropy_; // fa_entropy means to all the frontiers entropy
  visualization_msgs::MarkerArray marker_area_;
  visualization_msgs::Marker m_;
  double current_entropy_;
};
}
#endif
