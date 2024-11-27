#include <enum.h>
#include <framework.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <cassert>
#include <vector>
#include <string>
#include <math.h>
#include <utility>
#include <vector>
#include <queue>
#define no_assignment -1
enum {
	find_ingredient = 99,
	find_clean_plate ,
	send_recipe,
	take_dirtyplate, 
	clean_dirtyplate, 
	cleanning_now, 
	cook_pot, 
	cook_pan, 
	cook_chop, 
	chopping_now, 
	poting_now, 
	paning_now, 
	find_cook_way, 
	go_on_find_ingre,
	put_plate_down,
};
/* 按照读入顺序定义 */
int width, height;
char Map[20 + 5][20 + 5];
int IngredientCount;
struct Ingredient Ingredient[20 + 5];
int recipeCount;
struct Recipe Recipe[20 + 5];
int totalTime, randomizeSeed, totalOrderCount;
struct Order totalOrder[20 + 5];
int orderCount;
struct Order Order[20 + 5];
int k;
struct Player Players[2 + 5];
int entityCount;
struct Entity Entity[20 + 5];
int remainFrame, Fund;
std::pair<double, double> Pot_loc;
std::pair<double, double> Pan_loc;
int pot_idx, pan_idx;
void init()
{
	for (int j = 0; j < IngredientCount; j++)
	{
		Recipe[j + recipeCount].time = 0, Recipe[j + recipeCount].nameBefore = Ingredient[j].name;
		Recipe[j + recipeCount].kind = " ", Recipe[j + recipeCount].nameAfter = Ingredient[j].name;
		//把不需要烹饪的原料也算作Recipe
	}
	recipeCount += IngredientCount;
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].entity[0] == "Pot")
		{
			Pot_loc.first = Entity[i].x, Pot_loc.second = Entity[i].y;
			pot_idx = i;
		}		
		if (Entity[i].entity[0] == "Pan")
		{
			Pan_loc.first = Entity[i].x, Pan_loc.second = Entity[i].y;
			pan_idx = i;
		}
			
	}
}
void init_read()
{
	std::string s;
	std::stringstream ss;
	int frame;

	/* 读取初始地图信息 */
	std::getline(std::cin, s, '\0');
	ss << s;

	/* 若按照该读入，访问坐标(x, y)等价于访问Map[y][x],你可按照自己的习惯进行修改 */
	ss >> width >> height;
	std::cerr << "Map size: " << width << "x" << height << std::endl;
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			ss >> Map[i][j];

	/* 读入原料箱：位置、名字、以及采购单价 */
	ss >> IngredientCount;
	for (int i = 0; i < IngredientCount; i++)
	{
		ss >> s;
		assert(s == "IngredientBox");
		ss >> Ingredient[i].x >> Ingredient[i].y >> Ingredient[i].name >> Ingredient[i].price;
	}

	/* 读入配方：加工时间、加工前的字符串表示、加工容器、加工后的字符串表示 */
	ss >> recipeCount;
	for (int i = 0; i < recipeCount; i++)
	{
		ss >> Recipe[i].time >> Recipe[i].nameBefore >> Recipe[i].kind >> Recipe[i].nameAfter;
	}
	/* 读入总帧数、当前采用的随机种子、一共可能出现的订单数量 */
	ss >> totalTime >> randomizeSeed >> totalOrderCount;

	/* 读入订单的有效帧数、价格、权重、订单组成 */
	for (int i = 0; i < totalOrderCount; i++)
	{
		ss >> totalOrder[i].validFrame >> totalOrder[i].price >> totalOrder[i].frequency;
		getline(ss, s);
		std::stringstream tmp(s);
		while (tmp >> s)
			totalOrder[i].recipe.push_back(s);
	}

	/* 读入玩家信息：初始坐标 */
	ss >> k;
	assert(k == 2);
	for (int i = 0; i < k; i++)
	{
		ss >> Players[i].x >> Players[i].y;
		Players[i].containerKind = ContainerKind::None;
		Players[i].entity.clear();
	}

	/* 读入实体信息：坐标、实体组成 */
	ss >> entityCount;
	for (int i = 0; i < entityCount; i++)
	{
		ss >> Entity[i].x >> Entity[i].y >> s;
		Entity[i].entity.push_back(s);
	}
}

bool frame_read(int nowFrame)
{
	std::string s;
	std::stringstream ss;
	int frame;
	std::getline(std::cin, s, '\0');
	ss.str(s);
	/*
	  如果输入流中还有数据，说明游戏已经在请求下一帧了
	  这时候我们应该跳过当前帧，以便能够及时响应游戏。
	*/
	if (std::cin.rdbuf()->in_avail() > 0)
	{
		std::cerr << "Warning: skipping frame " << nowFrame
			<< " to catch up with the game" << std::endl;
		return true;
	}
	ss >> s;
	assert(s == "Frame");
	int currentFrame;
	ss >> currentFrame;
	assert(currentFrame == nowFrame);
	ss >> remainFrame >> Fund;
	/* 读入当前的订单剩余帧数、价格、以及配方 */
	ss >> orderCount;
	for (int i = 0; i < orderCount; i++)
	{
		ss >> Order[i].validFrame >> Order[i].price;
		Order[i].recipe.clear();
		getline(ss, s);
		std::stringstream tmp(s);
		while (tmp >> s)
		{
			Order[i].recipe.push_back(s);
		}
	}
	ss >> k;
	assert(k == 2);
	/* 读入玩家坐标、x方向速度、y方向速度、剩余复活时间 */
	for (int i = 0; i < k; i++)
	{
		ss >> Players[i].x >> Players[i].y >> Players[i].X_Velocity >> Players[i].Y_Velocity >> Players[i].live;
		getline(ss, s);
		std::stringstream tmp(s);
		Players[i].containerKind = ContainerKind::None;
		Players[i].entity.clear();
		//若玩家手里有东西，那么进行如下操作
		while (tmp >> s)
		{
			/*
				若若该玩家手里有东西，则接下来一个分号，分号后一个空格，空格后为一个实体。
				以下是可能的输入（省略前面的输入）
				 ;  : fish
				 ; @  : fish
				 ; @ Plate : fish
				 ; Plate
				 ; DirtyPlates 1
				...
			*/

			/* 若你不需要处理这些，可直接忽略 */
			if (s == ";" || s == ":" || s == "@" || s == "*")
				continue;

			/*
				Todo: 其他容器
			*/
			if (s == "Plate")
				Players[i].containerKind = ContainerKind::Plate;
			else if (s == "DirtyPlates")
				Players[i].containerKind = ContainerKind::DirtyPlates;
			/*
			my add level2
			*/
			else if (s == "Pan")
				Players[i].containerKind = ContainerKind::Pan;
			else if (s == "Pot")
				Players[i].containerKind = ContainerKind::Pot;
			/*
			end
			*/
			else
				Players[i].entity.push_back(s);
		}
	}

	ss >> entityCount;
	/* 读入实体坐标 */
	for (int i = 0; i < entityCount; i++)
	{
		ss >> Entity[i].x >> Entity[i].y;
		getline(ss, s);
		std::stringstream tmp(s);
		Entity[i].containerKind = ContainerKind::None;
		Entity[i].entity.clear();
		Entity[i].currentFrame = Entity[i].totalFrame = 0;
		Entity[i].sum = 1;
		while (tmp >> s)
		{
			/*
				读入一个实体，例子：
				DirtyPlates 2
				fish
				DirtyPlates 1 ; 15 / 180
				Plate : s_rice c_fish kelp
				Pot : s_rice ; 900 / 600
			*/

			/* 若你不需要处理这些，可直接忽略 */
			if (s == ":" || s == "@" || s == "*")
				continue;
			if (s == ";")
			{
				tmp >> Entity[i].currentFrame >> s >> Entity[i].totalFrame;
				assert(s == "/");
				break;
			}

			/*
				Todo: 其他容器
			*/
			if (s == "Plate")
				Entity[i].containerKind = ContainerKind::Plate;
			else if (s == "DirtyPlates")
			{
				Entity[i].containerKind = ContainerKind::DirtyPlates;
				tmp >> Entity[i].sum;
			}
			/*
			my add: level2
			*/
			else if (s == "Pot")
				Entity[i].containerKind = ContainerKind::Pot;
			else if (s == "Pan")
				Entity[i].containerKind = ContainerKind::Pan;
			/*
			end
			*/
			else
				Entity[i].entity.push_back(s);
		}
	}
	return false;
}
std::string player0_Action;
std::string player1_Action;
int dirx[4] = { -1, 0, 1, 0 };
int diry[4] = { 0, -1, 0, 1 };
std::vector<int> rec_item0;
std::vector<int> rec_item1;
int item_idx[2];
int target[2] = { no_assignment, no_assignment };
int order_target[2];
//int pot_use = -1, pan_use = -1;
std::queue<int> pot_use;
std::queue<int> pan_use;
void check_collision()
{
	//return;
	double px0 = Players[0].x, py0 = Players[0].y, px1 = Players[1].x, py1 = Players[1].y;
	if ((px0 - px1) * (px0 - px1) + (py0 - py1) * (py0 - py1) <= 1.5)
	{
		player1_Action = "Move";
		return;
		if (px0 < px1 && py0 < py1)
		{
			player1_Action += " D";
		}
		else if (px0 < px1 && py0 > py1)
		{
			player1_Action += " U";
		}
		else if (px0 > px1 && py0 > py1)
		{
			player1_Action += " D";
		}
		else if (px0 > px1 && py0 < py1)
		{
			player1_Action += " U";
		}
		/*if (px0 < px1)
			player1_Action += " R";
		else if (px0 >= px1)
			player1_Action += " L";
		if (py0 < py1)
			player1_Action += " D";
		else if (py0 >= py1)
			player1_Action += "U";*/
	}
}
bool Move(int x, int y, double px, double py, int which)
{
	//x和y传进来是map的整数坐标
	std::string Action = "Move";
	double dis = 0.37;
	bool x_ok = false, y_ok = false;
	double x1 = -1, y1 = -1;
	for (int i = 0; i < 4; i++)
	{
		if (y + diry[i] >= 0 && y + diry[i] < height && x + dirx[i] >= 0 && x + dirx[i] < width)
		{
			if (Map[y + diry[i]][x + dirx[i]] == '.')
			{
				x1 = (x + dirx[i]) + 0.5;
				y1 = (y + diry[i]) + 0.5;
				break;
			}
		}
	}
	assert(x1 != -1 && y1 != -1);
	if (px - x1 > dis)
	{
		//Left
		Action += " L";
	}
	else if (x1 - px > dis)
	{
		//Right
		Action += " R";
	}
	else
		x_ok = true;
	if (py - y1 > dis)
	{
		//Up
		Action += " U";
	}
	else if (y1 - py > dis)
	{
		//Down
		Action += " D";
	}
	else
		y_ok = true;
	if (which == 0)
		player0_Action = Action;
	else
	{
		player1_Action = Action;
		check_collision();
	}
	return x_ok && y_ok;
}
void PutOrPick(int x1, int y1, int px, int py, int which)
{
	std::string Action = "PutOrPick";
	if (y1 == py && x1 < px)
		Action += " L";
	else if (y1 == py && x1 > px)
		Action += " R";
	else if (x1 == px && y1 < py)
		Action += " U";
	else
		Action += " D";
	assert(Action != "PutOrPick");
	if (which == 0)
		player0_Action = Action;
	else
		player1_Action = Action;
}
void Interact(int x1, int y1, int px, int py, int which)
{
	std::string Action = "Interact";
	if (y1 == py && x1 < px)
		Action += " L";
	else if (y1 == py && x1 > px)
		Action += " R";
	else if (x1 == px && y1 < py)
		Action += " U";
	else
		Action += " D";
	assert(Action != "PutOrPick");
	if (which == 0)
		player0_Action = Action;
	else
		player1_Action = Action;
}
int check_clean_plate()
{
	//看看有没有干净的盘子，如果没有就不拿了
	int num = 0;
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].containerKind == ContainerKind::Plate && Entity[i].entity.size() == 0)
			num++;
	}
	return num;
}
std::pair<int, int> find_location(char ch)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (Map[i][j] == ch)
			{
				return std::pair<int, int>(i, j);
			}
		}
	}
	return std::pair<int, int>(0, 0);
}
void send_menu_step(int which)
{
	/*
	已经端上了盘子，并且集齐了食材，需要送到上菜口
	*/
	std::pair<int, int> loc = find_location('$');
	double sx = loc.second + 0.5, sy = loc.first + 0.5;
	if (Move(sx, sy, Players[which].x, Players[which].y, which) == true)
	{
		PutOrPick(sx, sy, Players[which].x, Players[which].y, which);
		target[which] = no_assignment;
	}
}
void get_item(std::string rec_name, int which)
{
	/*
	将订单切分成多个配方（按照顺序）
	*/
	//std::cerr << "name = " << rec_name << "\n";
	for (int i = 0; i < recipeCount; i++)
	{
		if (Recipe[i].nameAfter == rec_name)
		{
			if (which == 0)
				rec_item0.insert(rec_item0.begin(), i);
			else
				rec_item1.insert(rec_item1.begin(), i);
			for (int j = 0; j < IngredientCount; j++)
			{
				if (Recipe[i].nameBefore == Ingredient[j].name)
					return;
			}
			get_item(Recipe[i].nameBefore, which);
		}
	}
}
bool check_line(double x1, double y1)
{
	/*
	判断pan/pot/chop这些工作窗口是否正在工作
	*/
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].entity.size() != 0 && Entity[i].currentFrame > 0 && (int)Entity[i].x == (int)x1 && (int)Entity[i].y == (int)y1)
			return false;//说明已经有工作了，等待
	}
	return true;//没有工作，可以去
}
int rec_sum[2] = { 0, 0 };//表示这个是这一份订单的第几个菜品
int summ[2] = { 0, 0 };
//在最后target归-1的时候，重置为0
void assign(int which)
{
	/*
	分配合理的订单给玩家
	*/
	int start = 0;
	for (int i = 0; i < orderCount; i++)
	{
		//开始找匹配的菜谱
		//if()//条件优化
		if ((which == 0 && i == order_target[1]) || (which == 1 && i == order_target[0]))
			continue;
		{
			rec_sum[which] = 0;
			order_target[which] = i;
			if (which == 0)
				rec_item0.clear();//不晓得它的大小会不会变
			else
				rec_item1.clear();
			std::cerr << "====================== which = " <<which <<  "\n";
			std::cerr << "number = " << i << "\n";
			
			std::cerr << "initial size = " << Order[i].recipe.size() << "\n";
			summ[which] = Order[i].recipe.size();
			for (int j = 0; j < Order[i].recipe.size(); j++)
				std::cerr << "j = " << j << ", recipe name = " << Order[i].recipe[j] << "\n";
			get_item(Order[i].recipe[rec_sum[which]], which);
			target[which] = find_ingredient;
			item_idx[which] = 0;
			break;
		}
	}
}
void _put_plate_down(int which)
{
	//std::cerr << "which = " << which << "\n";
	assert(Players[which].containerKind == ContainerKind::Plate);
	//找到附近的一个空余的桌面放置
	for (int i = 0; i < height; i++)
	{
		for (int j = 1; j < width; j++)
		{
			if (Map[i][j] == '*')
			{
				double sx = j + 0.5, sy = i + 0.5;
				if (Move(sx, sy, Players[which].x, Players[which].y, which) == true)
				{
					PutOrPick(sx, sy, Players[which].x, Players[which].y, which);
					target[which] = go_on_find_ingre;
					std::cerr << "op" << "\n";
				}
				return;
			}
		}
	}
}
void _go_on_find_ingre(int which)
{
	if (which == 0)
		rec_item0.clear();
	else
		rec_item1.clear();
	get_item(Order[order_target[which]].recipe[rec_sum[which]], which);
	target[which] = find_ingredient;
	item_idx[which] = 0;
}
void _take_dirtyplate(int which)
{
	/*
	判断有脏碟子了，拿去洗手台
	*/
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].containerKind == ContainerKind::DirtyPlates)
		{
			if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
			{
				PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
				target[which] = clean_dirtyplate;
				//要去洗盘子了
			}
		}
	}
}
void _clean_dirtyplate(int which)
{
	/*
	按下J键，开始洗碗
	*/
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (Map[i][j] == 'k')
			{
				//找到了洗水池
				double sx = j + 0.5, sy = i + 0.5;
				if (Move(sx, sy, Players[which].x, Players[which].y, which) == true)
				{
					if (Players[which].containerKind != ContainerKind::None)
					{
						//手上拿了盘子，放下
						PutOrPick(sx, sy, Players[which].x, Players[which].y, which);
					}
					else
					{
						Interact(sx, sy, Players[which].x, Players[which].y, which);
						target[which] = cleanning_now;
					}
				}
				break;
			}
		}
	}
}
void _cleanning_now(int which)
{
	/*
	等待洗碗
	*/
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].containerKind == ContainerKind::DirtyPlates && Entity[i].currentFrame < Entity[i].totalFrame)
			break;
		if (i == entityCount - 1)
			target[which] = no_assignment;
	}
	
}
void judge_cook_kind(int which, int item)
{
	if (Recipe[item].kind == " ")
	{
		//表示不需要烹饪，直接拿盘子即可
		target[which] = find_clean_plate;
	}
	else
	{
		//通过kind找烹饪手法
		if (Recipe[item].kind == "-pot->")
		{
			target[which] = cook_pot;
			pot_use.push(which);
		}
		else if (Recipe[item].kind == "-pan->")
		{
			target[which] = cook_pan;
			pan_use.push(which);
		}
		else if (Recipe[item].kind == "-chop->")
		{
			//找切的地方（并且切是要一直守在旁边）
			target[which] = cook_chop;
			//可以拿盘子过来盛，也可以拿食材到盘子里（采用第二种）
		}
		else
			assert(0);
	}
}
void _find_ingredient(int which)
{
	/*
	找原材料，可能是ingredient里面的，也可能是加工后得到的
	*/
	bool find = false;
	int item = (which == 0) ? rec_item0[item_idx[0]] : rec_item1[item_idx[1]];
	for (int i = 0; i < IngredientCount && !find; i++)
	{
		if (Ingredient[i].name == Recipe[item].nameBefore)
		{
			find = true;
			//即这一个配方的东西是ingredient，可以直接拿
			if (Move(Ingredient[i].x + 0.5, Ingredient[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
			{
				PutOrPick(Ingredient[i].x + 0.5, Ingredient[i].y + 0.5, Players[which].x, Players[which].y, which);
				judge_cook_kind(which, item);
			}
		}
	}
	if (!find)
	{
		//说明是二次加工的并且已经拿在手上
		if (Players[which].entity.size() > 0 && Players[which].entity[0] == Recipe[item].nameBefore)
		{
			judge_cook_kind(which, item);
		}
	}
}
bool check_ingredient(int which, int plate_no)
{
	for (int i = 0; i < rec_sum[which]; i++)
	{
		for (int j = 0; j < Entity[plate_no].entity.size(); j++)
		{
			if (Entity[plate_no].entity[j] == Order[order_target[which]].recipe[i])
				break;
			if (j == Entity[plate_no].entity.size() - 1)
				return false;
		}
	}
	return true;
}
void _find_clean_plate(int which)
{
	/*
	找干净的盘子，判断此时是为何目的
	*/
	if (Players[which].containerKind == ContainerKind::Plate)
	{
		std::string kind;
		if(which == 0)
			kind = Recipe[rec_item0[item_idx[0]]].kind;
		else
			kind = Recipe[rec_item1[item_idx[1]]].kind;
		//std::cerr << "kind = " << kind << "\n";
		for (int i = 0; i < entityCount; i++)
		{
			if (Entity[i].containerKind == ContainerKind::Pot && kind == "-pot->")
			{
				if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true && Entity[i].currentFrame >= Entity[i].totalFrame)
				{
					PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
					rec_sum[which]++;
					std::cerr << "rec_sum = " << rec_sum[which] << ", Order_size = " << Order[order_target[which]].recipe.size() << "\n";
					if (rec_sum[which] < summ[which])
					{
						target[which] = put_plate_down;//先放下来
						std::cerr << "put downA" << "\n";
					}
					else
					{
						std::cerr << "pppp" << "\n";
						target[which] = send_recipe;
					}
					pot_use.pop();
				}
			}
			else if (Entity[i].containerKind == ContainerKind::Pan && kind == "-pan->")
			{
				if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true && Entity[i].currentFrame >= Entity[i].totalFrame)
				{
					PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
					rec_sum[which]++;
					if (rec_sum[which] < summ[which])
					{
						std::cerr << "put downB" << "\n";
						target[which] = put_plate_down;//先放下来
					}
					else
					{
						target[which] = send_recipe;
						std::cerr << "wwww" << "\n";
					}
					pan_use.pop();
				}
			}
		}
	}
	else
	{
		//std::cerr << "oppppppppppppp" << "\n";
		for (int i = 0; i < entityCount; i++)
		{
			if (Entity[i].containerKind == ContainerKind::Plate)
			{
				/*
				此时要判断这个盘子是否是干净的，在level3里面要判断有无东西
				TODO
				*/
				if (rec_sum[which] == 0)
				{
					//说明是第一份食材，可以随意找一个干净盘子
					if (Players[which].entity.size() == 0 && Players[which].containerKind == ContainerKind::None)
					{
						//表示现在手里没有东西，实际上是要把盘子拿过去
						if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
						{
							PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
							//assert(Players[0].containerKind == ContainerKind::Plate);
							//要到下一帧再拿
						}
					}
					else if (Players[which].entity.size() != 0)
					{
						//手上拿了东西
						if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
						{
							PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
							item_idx[which]++;
							rec_sum[which]++;
							if (rec_sum[which] < Order[order_target[which]].recipe.size())
							{
								target[which] = go_on_find_ingre;
								//std::cerr << "hshd" << "\n";
							}
							else
							{
								target[which] = send_recipe;
							}
						}
					}
				}
				else if (rec_sum[which] > 0)
				{
					//std::cerr << "have has thing" << "\n";
					//表示已经放了一些食材，这个时候要比对是否是之前已经放了东西的盘子
					if (check_ingredient(which, i) == true)
					{
						//std::cerr << "oooo" << "\n";
						//说明找到了这个盘子
						if (Players[which].entity.size() != 0)
						{
							//手上拿了东西
							if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
							{
								PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
								item_idx[which]++;
								rec_sum[which]++;
								if (rec_sum[which] < Order[order_target[which]].recipe.size())
								{
									target[which] = go_on_find_ingre;
								}
								else
								{
									target[which] = send_recipe;
								}
							}
						}
						else if (Players[which].entity.size() == 0 && Players[which].containerKind == ContainerKind::None)
						{
							//表示现在手里没有东西，实际上是要把盘子拿过去
							if (Move(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which) == true)
							{
								PutOrPick(Entity[i].x + 0.5, Entity[i].y + 0.5, Players[which].x, Players[which].y, which);
							}
						}
					}
				}
				break;
			}
		}
	}
	//表示已经取到所有食材，此时找干净盘子
	
}
void _send_recipe(int which)
{
	if (Players[which].containerKind == ContainerKind::None)
	{
		for (int i = 0; i < entityCount; i++)
		{
			if (Entity[i].containerKind == ContainerKind::Plate)
			{
				//这里可以加一个判断表示可以端走，即是recipe
				if (Entity[i].entity.size() < Order[order_target[which]].recipe.size())
					continue;
				bool flag = true;
				for (int j = 0; j < Entity[i].entity.size() && j < Order[order_target[which]].recipe.size(); j++)
				{
					if (Entity[i].entity[j] != Order[order_target[which]].recipe[j])
					{
						flag = false;
						break;
					}
				}//找到了正确的盘子，并将盘子拿了起来
				if (flag && Move(Entity[i].x, Entity[i].y, Players[which].x, Players[which].y, which) == true)
				{
					PutOrPick(Entity[i].x, Entity[i].y, Players[which].x, Players[which].y, which);
					break;
				}
				else
				{
					assert(0);
					//level3考虑的
				}
			}
		}
	}
	else
		send_menu_step(which);
}
void _cook_chop(int which)
{
	std::pair<int, int> loc = find_location('c');
	double sx = loc.second + 0.5, sy = loc.first + 0.5;
	//看是否有工作已经在这里进行，如果有，等待
	if (check_line(sx, sy) == true)
	{
		if (Move(sx, sy, Players[which].x, Players[which].y, which) == true)
		{
			if (Players[which].entity.size() > 0)
				PutOrPick(sx, sy, Players[which].x, Players[which].y, which);
			else
			{
				Interact(sx, sy, Players[which].x, Players[which].y, which);
				target[which] = chopping_now;
			}
		}
	}
}
void _chopping_now(int which)
{
	int item = (which == 0) ? rec_item0[item_idx[0]] : rec_item1[item_idx[1]];
	std::pair<int, int> loc = find_location('c');
	double sx = loc.second + 0.5, sy = loc.first + 0.5;
	for (int i = 0; i < entityCount; i++)
	{
		//此时表示切好了
		if ((int)sx == (int)Entity[i].x && (int)sy == (int)Entity[i].y && Entity[i].entity.size() >= 1 && Entity[i].entity[0] == Recipe[item].nameAfter && Entity[i].currentFrame >= Entity[i].totalFrame)
		{		
			//std::cerr << "which = " << which << "\n";
			if (Move(sx, sy, Players[which].x, Players[which].y, which) == true)
			{
				PutOrPick(sx, sy, Players[which].x, Players[which].y, which);
				item_idx[which]++;
				if (which == 0)
				{
					if (item_idx[0] >= rec_item0.size())
					{
						//说明找完了东西，可以去找盘子
						target[0] = find_clean_plate;
					}
					else
					{
						//重新找材料
						target[0] = find_ingredient;
					}
				}
				else
				{
					if (item_idx[1] >= rec_item1.size())
						target[1] = find_clean_plate;
					else
						target[1] = find_ingredient;
				}
				break;
			}
		}
	}
}
void _cook_pot_or_pan(int which)
{
	for (int i = 0; i < entityCount; i++)
	{
		if ((Entity[i].containerKind == ContainerKind::Pot && target[which] == cook_pot && pot_use.front() == which) || (Entity[i].containerKind == ContainerKind::Pan && target[which] == cook_pan && pan_use.front() == which))
		{
			if (Move(Entity[i].x, Entity[i].y, Players[which].x, Players[which].y, which) == true)
			{
				if (Players[which].entity.size() > 0)
				{
					PutOrPick(Entity[i].x, Entity[i].y, Players[which].x, Players[which].y, which);
					target[which] = find_clean_plate;
				}
				else
				{
					Interact(Entity[i].x, Entity[i].y, Players[which].x, Players[which].y, which);
					target[which] = find_clean_plate;
					
				}
			}
		}
	}
}
int check_dirty_plate()
{
	int num = 0;
	for (int i = 0; i < entityCount; i++)
	{
		if (Entity[i].containerKind == ContainerKind::DirtyPlates)
			num++;
	}
	return num;
}
void strategy()
{
	player0_Action = "";
	player1_Action = "";
	/*
	Player0 ////////////////////
	*/
	if (Players[0].containerKind == ContainerKind::None && target[0] == no_assignment && check_clean_plate())
	{
		assign(0);
	}
	/*else if (Players[0].containerKind == ContainerKind::None && target[0] == no_assignment && check_clean_plate() == 0)
	{
		target[0] = take_dirtyplate;
	}
	else if (target[0] == take_dirtyplate)
	{
		//表示要拿起脏盘子到洗水池清洗
		_take_dirtyplate(0);
	}
	else if (target[0] == clean_dirtyplate)
	{
		//找洗水池
		_clean_dirtyplate(0);
	}
	else if (target[0] == cleanning_now)
	{
		_cleanning_now(0);
	}*/
	else if (target[0] == find_ingredient)
	{
		_find_ingredient(0);
	}
	else if (target[0] == find_clean_plate)
	{
		_find_clean_plate(0);
	}
	else if (target[0] == go_on_find_ingre)
	{
		_go_on_find_ingre(0);
	}
	//此时可以送菜了，并看四周走一圈看是否有东西
	else if (target[0] == send_recipe)
	{
		_send_recipe(0);
	}
	else if (target[0] == cook_chop)
	{
		_cook_chop(0);
	}
	else if (target[0] == chopping_now)
	{
		_chopping_now(0);
	}
	else if (target[0] == cook_pot || target[0] == cook_pan)
	{
		_cook_pot_or_pan(0);
	}
	else if (target[0] == put_plate_down)
	{
		_put_plate_down(0);
	}

	///////////////////////////////////////
	/*if (target[1] == no_assignment && check_dirty_plate() > 0)
	{
		//std::cerr << "clean" << "\n";
		target[1] = take_dirtyplate;
	}
	else if (target[1] == take_dirtyplate)
	{
		//表示要拿起脏盘子到洗水池清洗
		_take_dirtyplate(1);
	}
	else if (target[1] == clean_dirtyplate)
	{
		//找洗水池
		_clean_dirtyplate(1);
	}
	else if (target[1] == cleanning_now)
	{
		_cleanning_now(1);
	}*/
	if (Players[1].containerKind == ContainerKind::None && target[1] == no_assignment && check_clean_plate() > 1)
	{
		assign(1);
	}
	else if (target[1] == no_assignment && check_dirty_plate() > 0)
	{
		//std::cerr << "clean" << "\n";
		target[1] = take_dirtyplate;
	}
	else if (target[1] == take_dirtyplate)
	{
		//表示要拿起脏盘子到洗水池清洗
		_take_dirtyplate(1);
	}
	else if (target[1] == clean_dirtyplate)
	{
		//找洗水池
		_clean_dirtyplate(1);
	}
	else if (target[1] == cleanning_now)
	{
		_cleanning_now(1);
	}
	else if (target[1] == find_ingredient)
	{
		_find_ingredient(1);
	}
	else if (target[1] == find_clean_plate)
	{
		_find_clean_plate(1);
	 }
	else if (target[1] == go_on_find_ingre)
	{
		_go_on_find_ingre(1);
	}
	//此时可以送菜了，并看四周走一圈看是否有东西
	else if (target[1] == send_recipe)
	{
		_send_recipe(1);
	}
	else if (target[1] == cook_chop)
	{
		_cook_chop(1);
	}
	else if (target[1] == chopping_now)
	{
		_chopping_now(1);
	}
	else if (target[1] == cook_pot || target[1] == cook_pan)
	{
		_cook_pot_or_pan(1);
	}
	else if (target[1] == put_plate_down)
	{
		_put_plate_down(1);
	}
	//player1_Action = "Move U";
	std::string action = player0_Action + "\n" + player1_Action + "\n";
	std::cout << action;
}
