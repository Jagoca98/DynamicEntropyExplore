#include <explore/frontier_search.h>

#include <mutex>

#include <costmap_2d/cost_values.h>
#include <costmap_2d/costmap_2d.h>
#include <geometry_msgs/Point.h>

#include <explore/costmap_tools.h>

#include <tgmath.h>

namespace frontier_exploration
{
using costmap_2d::LETHAL_OBSTACLE;
using costmap_2d::NO_INFORMATION;
using costmap_2d::FREE_SPACE;

FrontierSearch::FrontierSearch(costmap_2d::Costmap2D* costmap,
                               double potential_scale, double gain_scale,
                               double min_frontier_size, double expected_radius, 
                               double gain_entropy, double obstacle_gain)
  : costmap_(costmap)
  , potential_scale_(potential_scale)
  , gain_scale_(gain_scale)
  , min_frontier_size_(min_frontier_size)
  , expected_radius_(expected_radius)
  , gain_entropy_(gain_entropy)
  , obstacle_gain_(obstacle_gain)
{
}

std::vector<Frontier> FrontierSearch::searchFrom(geometry_msgs::Point position)
{
  std::vector<Frontier> frontier_list;

  // Sanity check that robot is inside costmap bounds before searching
  unsigned int mx, my;
  if (!costmap_->worldToMap(position.x, position.y, mx, my)) {
    ROS_ERROR("Robot out of costmap bounds, cannot search for frontiers");
    return frontier_list;
  }

  // make sure map is consistent and locked for duration of search
  std::lock_guard<costmap_2d::Costmap2D::mutex_t> lock(*(costmap_->getMutex()));

  map_ = costmap_->getCharMap();
  size_x_ = costmap_->getSizeInCellsX();
  size_y_ = costmap_->getSizeInCellsY();

  // initialize flag arrays to keep track of visited and frontier cells
  std::vector<bool> frontier_flag(size_x_ * size_y_, false);
  std::vector<bool> visited_flag(size_x_ * size_y_, false);

  // initialize breadth first search
  std::queue<unsigned int> bfs;

  // find closest clear cell to start search
  unsigned int clear, pos = costmap_->getIndex(mx, my);
  
  if (nearestCell(clear, pos, FREE_SPACE, *costmap_)) {
    bfs.push(clear);
  } else {
    bfs.push(pos);
    ROS_WARN("Could not find nearby clear cell to start search");
  }
  visited_flag[bfs.front()] = true;

  while (!bfs.empty()) {
    unsigned int idx = bfs.front();
    bfs.pop();

    if(map_[idx] == LETHAL_OBSTACLE || map_[idx] == FREE_SPACE){
      gknown_cells_.push(idx);
    }
    // iterate over 4-connected neighbourhood
    for (unsigned nbr : nhood4(idx, *costmap_)) {
      // add to queue all free, unvisited cells, use descending search in case
      // initialized on non-free cell
      if (map_[nbr] <= map_[idx] && !visited_flag[nbr]) {
        visited_flag[nbr] = true;
        bfs.push(nbr);
        // check if cell is new frontier cell (unvisited, NO_INFORMATION, free
        // neighbour)
      } else if (isNewFrontierCell(nbr, frontier_flag)) {
        frontier_flag[nbr] = true;
        Frontier new_frontier = buildNewFrontier(nbr, pos, frontier_flag);
        if (new_frontier.size * costmap_->getResolution() >=
            min_frontier_size_) {
          frontier_list.push_back(new_frontier);
        }
      }
    }
  }
  //clear the r-map
  puntos.clear();
  points.clear();
  // set costs of frontiers
  // current_entropy_ = currentEntropy();
  for (auto& frontier : frontier_list) {
    frontier.entropy_gain = frontierEntropyGain(frontier);
    frontier.cost = frontierCost(frontier);
  }

  std::sort(
      frontier_list.begin(), frontier_list.end(),
      [](const Frontier& f1, const Frontier& f2) { return f1.cost < f2.cost; });

  return frontier_list;
}

Frontier FrontierSearch::buildNewFrontier(unsigned int initial_cell,
                                          unsigned int reference,
                                          std::vector<bool>& frontier_flag)
{
  // initialize frontier structure
  Frontier output;
  output.centroid.x = 0;
  output.centroid.y = 0;
  output.size = 1;
  output.min_distance = std::numeric_limits<double>::infinity();
  output.known_cells = 0;
  output.unknown_cells = 0;
  output.goal.x = 0;
  output.goal.y = 0;


  // record initial contact point for frontier
  unsigned int ix, iy;
  costmap_->indexToCells(initial_cell, ix, iy);
  costmap_->mapToWorld(ix, iy, output.initial.x, output.initial.y);

  // push initial gridcell onto queue
  std::queue<unsigned int> bfs;
  bfs.push(initial_cell);

  // cache reference position in world coords
  unsigned int rx, ry;
  double reference_x, reference_y;
  costmap_->indexToCells(reference, rx, ry);
  costmap_->mapToWorld(rx, ry, reference_x, reference_y);
  
  std::vector<unsigned int> index;
  index.push_back(initial_cell);

  while (!bfs.empty()) {
    unsigned int idx = bfs.front();
    bfs.pop();

    // try adding cells in 8-connected neighborhood to frontier
    for (unsigned int nbr : nhood8(idx, *costmap_)) {
      
      // Recount of known and uknown cells in the surroundings
      if(map_[nbr] == LETHAL_OBSTACLE || map_[nbr] == FREE_SPACE){
        output.known_cells++;
      }
      else{
        output.unknown_cells++;
      }


      // check if neighbour is a potential frontier cell
      if (isNewFrontierCell(nbr, frontier_flag)) {

        // mark cell as frontier
        frontier_flag[nbr] = true;
        unsigned int mx, my;
        double wx, wy;
        costmap_->indexToCells(nbr, mx, my);
        costmap_->mapToWorld(mx, my, wx, wy);

        geometry_msgs::Point point;
        point.x = wx;
        point.y = wy;
        output.points.push_back(point);

        // update frontier size
        output.size++;

        // update centroid of frontier
        output.centroid.x += wx;
        output.centroid.y += wy;

        // determine frontier's distance from robot, going by closest gridcell
        // to robot
        double distance = sqrt(pow((double(reference_x) - double(wx)), 2.0) +
                               pow((double(reference_y) - double(wy)), 2.0));
        if (distance < output.min_distance) {
          output.min_distance = distance;
          output.middle.x = wx;
          output.middle.y = wy;
        }

        // add to queue for breadth first search
        bfs.push(nbr);
        index.push_back(nbr);
      }
    }
  }

  // average out frontier centroid
  output.centroid.x /= output.size;
  output.centroid.y /= output.size;

  unsigned int mx, my;
  double wx, wy;
  costmap_->indexToCells(index[(int) index.size()/2], mx, my);
  costmap_->mapToWorld(mx, my, wx, wy);
  output.goal.x = (output.centroid.x + wx)/2;
  output.goal.y = (output.centroid.y + wy)/2;
  
  return output;
}

bool FrontierSearch::isNewFrontierCell(unsigned int idx,
                                       const std::vector<bool>& frontier_flag)
{
  // check that cell is unknown and not already marked as frontier
  if (map_[idx] != NO_INFORMATION || frontier_flag[idx]) {
    return false;
  }

  // frontier cells should have at least one cell in 4-connected neighbourhood
  // that is free
  for (unsigned int nbr : nhood4(idx, *costmap_)) {
    if (map_[nbr] == FREE_SPACE) {
      return true;
    }
  }

  return false;
}

double FrontierSearch::frontierEntropyGain(Frontier& frontier){
  double entropy_gain = 0;
  double factor;
  int f_expected_known; // Expected known cells
  double f_expected_unknown = f_expected_ucells(frontier, f_expected_known, factor);    
  entropy_gain = factor * frontier.f_entropy;
  return entropy_gain;
}

int FrontierSearch::f_expected_ucells(Frontier& frontier, int &expected_kcells, double &factor){
  
  int expected_ucells = 0;   // Inicializamos la variable
  int unknown_cells = 0;
  int known_free_cells = 0.0;
  int known_nonfree_cells = 0;

  std::queue<unsigned int> bfs; // Iniciamos el breadth first search
  std::vector<bool> visited_flag(size_x_ * size_y_, false);    // Flags
  double r = expected_radius_;

  unsigned int mx, my;

  double wxo{frontier.points[0].x}, wyo{frontier.points[0].y};
  if (!costmap_->worldToMap(wxo, wyo, mx, my)) {
    ROS_ERROR("Boundary is out of size");
  }

  unsigned int inicial_cell = costmap_->getIndex(mx, my); 
  bfs.push(inicial_cell);
  
  unsigned int mxbr, mybr;
  double xwbr, ywbr;

  map_ = costmap_->getCharMap();

  while(!bfs.empty()){
    unsigned int idx = bfs.front();
    bfs.pop();

    for(int nbr : nhood8(idx, *costmap_)){
      costmap_->indexToCells(nbr, mxbr, mybr);
      costmap_->mapToWorld(mxbr, mybr, xwbr, ywbr);

      double distance_min = abs(xwbr- frontier.centroid.x) + abs(ywbr- frontier.centroid.y); 
      // ROS_DEBUG("Distance = %f", distance_min);
      // Distancia minima a frontera
      for (size_t j=0; j<frontier.points.size();j++){
        double distance = abs(xwbr- frontier.points[j].x) + abs(ywbr- frontier.points[j].y);
        // ROS_INFO("Distance = %f", distance);
        if(distance_min > distance) distance_min=distance; 
        // ROS_INFO("Distance min = %f", distance_min);
      } 
      // ROS_INFO("Distance min = %f", distance_min);
      if(distance_min < r  && !(visited_flag[nbr] == true)){
        if(map_[nbr] == FREE_SPACE && !visited_flag[nbr]){
          known_free_cells++;
        }
        if(map_[nbr] == LETHAL_OBSTACLE && !visited_flag[nbr]){
          known_nonfree_cells++;
        }
        if(map_[nbr] == NO_INFORMATION && !visited_flag[nbr]){
          unknown_cells++;
          geometry_msgs::Point p;
          punto point;
          point.puntos.x = xwbr;
          point.puntos.y = ywbr;
          point.valor = std::min(distance_min/(2*expected_radius_), 0.5);
          p.x = xwbr;
          p.y = ywbr;
          frontier.puntos.push_back(point);
          points.push_back(p);
          puntos.push_back(point);
        }

        visited_flag[nbr]=true;
        bfs.push(nbr);
      }
    }
  }
  double P_obstacle = (obstacle_gain_ * (double)known_nonfree_cells)/(known_nonfree_cells+known_free_cells);
  expected_kcells = unknown_cells;
  factor = (1-P_obstacle);
  ROS_INFO("f: %f", factor);
  // Estimamos el arco de la circunferencia como la proporción de desconocidas
  expected_ucells = (double)expected_kcells/(expected_kcells+known_nonfree_cells+known_free_cells)*2*M_PI*expected_radius_;
  
  // Calculamos la entropia de la frontera
  frontier.f_entropy = 0;
  for(size_t j = 0; j<frontier.puntos.size(); j++){
    double value = frontier.puntos[j].valor;
    if(value>0 && value<1){
      frontier.f_entropy += -(value*log2(value)+(1-value)*log2(1-value));
    }
  }

  return expected_ucells;
}

double FrontierSearch::currentEntropy(void){
  double current_entropy;
  double g_known = (double)gknown_cells_.size();
  g_unknown_ = g_unknown; 
  fa_entropy_ = fa_entropy;
  double g_all = g_known + g_unknown_;
  double P_known = (g_known);
  // current_entropy = -(P_known*log2(P_known)+(1-P_known)*log2(1-P_known));
  current_entropy = costmap_->getSizeInCellsX() * costmap_->getSizeInCellsY() - g_known + fa_entropy_;

  return  current_entropy;
}

double FrontierSearch::frontierCost(const Frontier& frontier)
{
  ROS_INFO("Distance: %f", frontier.min_distance);
  ROS_INFO("Infogain: %f", frontier.entropy_gain);
  return (potential_scale_ * frontier.min_distance *
          costmap_->getResolution()) -
         (gain_scale_ * frontier.size * costmap_->getResolution() + gain_entropy_ * frontier.entropy_gain);
}
}
