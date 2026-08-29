/*
 * 收藏功能页面（移植自同事项目 screen_Favorites.c，按我方工程适配）
 * 独立页面：不依赖 ui_manager / ui_objects.h，全部符号见 nav_favorites.h
 * 页面创建 screen_favorites_create / 业务逻辑 / 导航壳 nav_favorites.c
 */
#include "nav_favorites.h"
#include "nav_lang.h"
#include <string.h>


SUM_favorites_Value Func_favorites_Value = 				//收藏夹的内容值（非探针模式使用）
{
    .has_favorites_byte = 0x00,
    .favorites_val = {
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第一个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第二个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第三个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第四个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第五个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第六个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第七个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0} 		// 第八个收藏
    }
};
SUM_favorites_Value Func_favorites_Value_Probe = 			//探针模式收藏夹的内容值（is_probe_inserted() 时使用）
{
    .has_favorites_byte = 0x00,
    .favorites_val = {
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第一个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第二个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第三个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第四个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第五个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第六个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第七个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0} 		// 第八个收藏
    }
};
SUM_favorites_Value Func_favorites_Value_NULL = 		//空收藏夹的内容值
{
    .has_favorites_byte = 0x00,
    .favorites_val = {
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第一个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第二个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第三个收藏（默认值）
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第四个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第五个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第六个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0},		// 第七个收藏
        {0, 0, 0, 0, 0, -1, 0, -1, 0, 0, 0, 0} 		// 第八个收藏
    }
};

//当前使用的收藏集合指针：探针模式(is_probe_inserted())指向探针收藏，否则指向普通收藏
SUM_favorites_Value *Fav_Cur = &Func_favorites_Value;

//根据外部 Probe_Mode 选择当前收藏集合，进入收藏界面或读写收藏前调用
void Fav_Select_By_Probe(void)
{
	Fav_Cur = (is_probe_inserted()) ? &Func_favorites_Value_Probe : &Func_favorites_Value;
}

Fun_Multi_SUM_Value Favorites_Value	= {0};			//收藏夹中多段烹饪的参数
//int8_t Fav_probe_Fun[7] = {-1, -1, -1, -1, -1, -1, -1};

Fav_lb_str Fav_line_lb_str = {""};

uint8_t favorites_how_many = 0;				//收藏夹内有多少功能
int8_t favorites_choose_which = 1;			//选择哪一个收藏功能

uint8_t  input_Cooking_Mode = 0;	//收藏参数
uint16_t input_Temp 	= 0;		//收藏参数
int8_t 	 input_Hour 	= 0;		//收藏参数
int8_t 	 input_Minute 	= 0;		//收藏参数
int8_t 	 input_Is_Steam = 0;		//收藏参数
int8_t 	 input_Mode_name = 0;		//收藏参数
int8_t 	 input_Six_num  = 0;		//收藏参数

uint16_t input_Temp_Conventional_Dowm 	 = 0;		//收藏 上下烧烤 下腔温度 参数
uint8_t  input_Temp_probe[FAV_FUNLIST_COUNT];		//收藏 探针模式 探针温度 参数

int8_t Del_Fav_create_flag = 0;		//创建删除收藏夹的界面标志位，1：创建删除
uint8_t fav_succeed_no_repetitive = 0;	//收藏成功且不重复标志

void FAV_screen_Refresh_FirstPage();
void FAV_screen_Refresh_SecondPage();
void FAV_Option_SteamGear(lv_obj_t *obj, int8_t Steam_Gear);
void FAV_Option_LB_str(lv_obj_t *obj, int8_t Cooking_FUN);


//整体安全的删除组
void favo_safety_group_delete()
{
    screen_favorites_t *scr = &g_fav_screen;

    /* 悬空指针防御（闪退根因）：删除组前先断开所有外部引用。
     * 组被 lv_group_del 释放后，若 g_favorites/current_group/default 仍指向它，
     * 新页面组建组 malloc 复用同地址会使 current_group==g_favorites 误命中，
     * 编码器误入收藏分支对 NULL 组 focus_next 直接断言闪退 */
    lv_group_t *fav_def = lv_group_get_default();
    if (fav_def == scr->group || fav_def == scr->group_sub_1 || fav_def == scr->group_sub_2 ||
        fav_def == scr->group_sub_3 || fav_def == scr->group_sub_4)
        lv_group_set_default(NULL);
    if (current_group == scr->group || current_group == scr->group_sub_1 ||
        current_group == scr->group_sub_2 || current_group == scr->group_sub_3 ||
        current_group == scr->group_sub_4)
        current_group = NULL;
    g_favorites = NULL;

    //*防御检测是否为空组
    if (scr->group == NULL) printf("[fav] group null\n");
    else 
    {
        lv_group_del(scr->group);
        scr->group = NULL;
    }
    if (scr->group_sub_1 == NULL) printf("[fav] group_sub_1 null\n");
    else 
    {
        lv_group_del(scr->group_sub_1);
        scr->group_sub_1 = NULL;
    }
    if (scr->group_sub_2 == NULL) printf("[fav] group_sub_2 null\n");
    else 
    {
        lv_group_del(scr->group_sub_2);
        scr->group_sub_2 = NULL;
    }
    if (scr->group_sub_3 == NULL) printf("[fav] group_sub_3 null\n");
    else 
    {
        lv_group_del(scr->group_sub_3);
        scr->group_sub_3 = NULL;
    }
    if (scr->group_sub_4 == NULL) printf("[fav] group_sub_4 null\n");
    else 
    {
        lv_group_del(scr->group_sub_4);
        scr->group_sub_4 = NULL;
    }
}


// 检测最低位0的位置
int8_t find_lowest_bit_zero_position(uint8_t num) {
    for (int8_t i = 0; i < 8; i++) {
        if (!(num & (1 << i))) {
            return i;
        }
    }
    return -1; // 所有位都是1
}

/**
 * @brief 解析 Six 菜单收藏卡片的显示内容
 *        从菜谱模板(L1/L2/L3)中找到 "小结：" 正文，按行拆分为 summary1/summary2；
 *        同时计算烹饪时间文本与是否含 "预计烹饪时间：" 模板
 * @param fav     收藏项
 * @param summary1 [out] 小结第一行（无小结时为空串）
 * @param s1_sz    summary1 缓冲区大小
 * @param summary2 [out] 小结第二行（无第二行时为空串）
 * @param s2_sz    summary2 缓冲区大小
 * @param lines    [out] 小结行数（0=无小结，1=单行，2=两行）
 * @param time_buf [out] 烹饪时间文本，如 "时间：1小时30分钟"
 * @param t_sz     time_buf 缓冲区大小
 * @param has_cook_time [out] 模板中是否含 "预计烹饪时间："
 */
static void FAV_Six_Parse(const Fun_favorites_Value *fav, char *summary1, int s1_sz,
                          char *summary2, int s2_sz, int *lines,
                          char *time_buf, int t_sz, int *has_cook_time)
{
	/* 临时切换到收藏保存的菜谱，读取名称/时间后恢复（我方 six_bread_cfg 表驱动） */
	uint8_t old_fun = g_six_bread_type;
	g_six_bread_type = (uint8_t)fav->Six_Cook_Fun;

	snprintf(summary1, s1_sz, "%s", six_bread_name());

	int total = six_bread_cook_min();
	if (fav->Six_FaJiao)
		total += 45;
	if (total >= 60)
		snprintf(summary2, s2_sz, "%s:%s %s:%d%s%d%s",
		         tr("发酵"), fav->Six_FaJiao ? tr("是") : tr("否"),
		         tr("时间"), total / 60, tr("小时"), total % 60, tr("分钟"));
	else
		snprintf(summary2, s2_sz, "%s:%s %s:%d%s",
		         tr("发酵"), fav->Six_FaJiao ? tr("是") : tr("否"),
		         tr("时间"), total, tr("分钟"));
	*lines = 2;
	*has_cook_time = 0;
	time_buf[0] = '\0';

	g_six_bread_type = old_fun;
}
static void FAV_Set_FuncTemp_Lb(lv_obj_t *lb, const Fun_favorites_Value *fav)
{
	printf("[fav-set] FuncTemp obj=%p mode=%d temp=%d down=%d probe=%d\n",
	       lb, fav->PengTiaoMode_name, fav->temperature, input_Temp_Conventional_Dowm, fav->Probe_temp);
	if (fav->PengTiaoMode_name == FAV_MODE_MULTI || fav->PengTiaoMode_name == FAV_MODE_SIX)
		lv_label_set_text(lb, " ");
	else if (fav->PengTiaoMode_name == MODE_UPDOWN_BBQ)
		if (is_probe_inserted()) lv_label_set_text_fmt(lb, "%d℃", fav->temperature);
		else lv_label_set_text_fmt(lb, "↑%d℃/↓%d℃", fav->temperature,
		                           (fav->temp_down > 0) ? fav->temp_down : fav->temperature);	//↓按卡读取（此前读全局残留值，多卡片串值；旧数据 temp_down=0 回退上温）
	else
		lv_label_set_text_fmt(lb, "%d℃", fav->temperature);
	printf("[fav-set] FuncTemp txt=%s\n", lv_label_get_text(lb));
}
static void FAV_Set_L1_Lb(lv_obj_t *lb, const Fun_favorites_Value *fav)
{
	if (fav->PengTiaoMode_name == FAV_MODE_SIX)
	{
		char s1[256], s2[256], tbuf[64];
		int lines, has_cook_time;
		FAV_Six_Parse(fav, s1, sizeof(s1), s2, sizeof(s2), &lines,
		              tbuf, sizeof(tbuf), &has_cook_time);
		if (lines >= 1)
			lv_label_set_text(lb, s1);
		else
			lv_label_set_text(lb, " ");
	}
	else
	{
		FAV_Option_LB_str(lb, fav->PengTiaoMode_name);
	}
}
static void FAV_Set_L2_Lb(lv_obj_t *lb, const Fun_favorites_Value *fav)
{
	if (fav->PengTiaoMode_name == FAV_MODE_SIX)
	{
		char s1[256], s2[256], tbuf[64];
		int lines, has_cook_time;
		FAV_Six_Parse(fav, s1, sizeof(s1), s2, sizeof(s2), &lines,
		              tbuf, sizeof(tbuf), &has_cook_time);
		if (lines >= 2)
			lv_label_set_text(lb, s2);
		else
			lv_label_set_text(lb, " ");
	}
	else if (fav->Probe_temp && Fav_Cur == &Func_favorites_Value_Probe)	//仅探针收藏集合显示（旧数据普通收藏误存 80，一并屏蔽）
	{
		lv_label_set_text_fmt(lb, tr("探针温度：%d℃"), fav->Probe_temp);
	}
	else
	{
		printf("[fav-set] L2 obj=%p mode=%d h=%d m=%d probe=%d\n",
		       lb, fav->PengTiaoMode_name, fav->Func_Hour, fav->Func_Minute, fav->Probe_temp);
		FAV_Option_LB_str(lb, fav->PengTiaoMode_name);
		printf("[fav-set] L2 txt=%s\n", lv_label_get_text(lb));
	}
}
void Input_favorites(uint16_t temp, int8_t Hour, int8_t Minute, int8_t Is_Steam, int8_t Mode_name, uint8_t Cooking_Mode) 
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
    int8_t set_place = find_lowest_bit_zero_position(Fav_Cur->has_favorites_byte);
    if (set_place == -1) return; // 无空位

    Fun_favorites_Value *item = &Fav_Cur->favorites_val[set_place];
    item->temperature = temp;
    item->Func_Hour = Hour;
    item->Func_Minute = Minute;
    item->Func_Steam = Is_Steam;
    item->PengTiaoMode_name = Mode_name;
	item->Probe_temp = is_probe_inserted() ? input_Temp_probe[0] : 0;	//无探针不存探针温度（probe_target_temp 默认 80，防卡片误显示）
	item->PengTiao_Mode = Cooking_Mode;
	item->temp_down = input_Temp_Conventional_Dowm;	//下腔温度随收藏保存（此前漏存：启动读到 0 回退主温、卡片只能读全局残留值）

	if (Mode_name == FAV_MODE_SIX)
	{
		item->Six_Cook_Fun = g_six_bread_type;
		item->Six_KaoSe = (int8_t)toastcolor_degree_value();
		item->Six_FaJiao = (g_rising_choice == 1) ? 1 : 0;
		item->Six_KG = (int8_t)toastcolor_weight_value();
		item->Six_Maturity = (int8_t)six_maturity_idx();
	}
	item->source_page = g_delay_source_page;

    Fav_Cur->has_favorites_byte |= (1 << set_place);
}

/**
 * @brief 删除指定位的收藏夹
 * 
 * @param favorites_id 收藏夹位的id
 */
void Delete_favorites(uint16_t favorites_id)
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	while (Fav_Cur->has_favorites_byte & (0b10 << favorites_id))
	{
		Fav_Cur->favorites_val[favorites_id] = Fav_Cur->favorites_val[favorites_id + 1];

		favorites_id ++;
	}
	
	Fav_Cur->favorites_val[favorites_id] = Func_favorites_Value_NULL.favorites_val[favorites_id];
	// 清除最高位1
	Fav_Cur->has_favorites_byte &= ~(1 << favorites_id);

	favorites_how_many--;
	printf("\nfavor  %d\n", favorites_how_many);
}

//清空全部收藏夹
void Clear_all_favorites(void)
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	*Fav_Cur = Func_favorites_Value_NULL;
	favorites_how_many = 0;
	printf("All favorites cleared.\n");
}

//打印收藏夹的状态
void print_status(const SUM_favorites_Value* manager) {
    printf("Status byte: 0x%02X\n", manager->has_favorites_byte);
    printf("Machine status:\n");
    for (uint8_t i = 0; i < 8; i++) {
        printf("favorites %d: Temp=%d,  Hour=%d,  Minute=%d,  Steam=%d,  Name=%d \n", 
               i, 
               manager->favorites_val[i].temperature, 
               manager->favorites_val[i].Func_Hour,
			   manager->favorites_val[i].Func_Minute,
			   manager->favorites_val[i].Func_Steam,
			   manager->favorites_val[i].PengTiaoMode_name);
    }
    printf("\n");
}

//添加一个普通功能收藏
void Add_favorites()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	if (is_probe_inserted())	//探针模式下不用收藏时间
    {
		Input_favorites(input_Temp, 0, 0, input_Is_Steam, input_Mode_name, input_Cooking_Mode);
	}
	else if (input_Mode_name == FAV_MODE_SIX)
	{
		Input_favorites(0, input_Hour, input_Minute, 0, input_Mode_name, input_Cooking_Mode);
	}
	else					//普通模式
	{
		Input_favorites(input_Temp, input_Hour, input_Minute, input_Is_Steam, input_Mode_name, input_Cooking_Mode);
	}
	print_status(Fav_Cur);
}
//添加一个多段烹饪功能收藏
void Add_favorites_of_Multi(Fun_Multi_SUM_Value Fav_Val)
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	int8_t set_place = find_lowest_bit_zero_position(Fav_Cur->has_favorites_byte);
    if (set_place == -1) return; // 无空位

	Fav_Cur->favorites_val[set_place].PengTiaoMode_name = FAV_MODE_MULTI;
    Fav_Cur->has_favorites_byte |= (1 << set_place);
	Favorites_Value = Fav_Val;

	//保存首段通信模式编号，供启动恢复 PengTiao_mode_num
	Fav_Cur->favorites_val[set_place].PengTiao_Mode = Favorites_Value.Func_Value_step_1.PengTiaoMode_num;

	print_status(Fav_Cur);
}
//检测收藏是否是已存在功能，0：不存在，1：存在
bool Favorites_Check_Exists()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
    for (uint8_t i = 0; i < 8; i++)
	{
        if (input_Mode_name == Fav_Cur->favorites_val[i].PengTiaoMode_name)
		{
			if (Fav_Cur->favorites_val[i].PengTiaoMode_name == FAV_MODE_SIX)	// 如果是第六感菜单
			{
				if (Fav_Cur->favorites_val[i].Six_Cook_Fun == input_Six_num) // 校验当前第六感菜谱是否已有
				{
					return true;
				}
				else
				{
					continue;
				}
			}
			else if (is_probe_inserted())
			{
				if (Fav_Cur->favorites_val[i].Probe_temp)
				{
					return true;
				}
				else
				{
					continue;
				}
			}
			else if (Fav_Cur->favorites_val[i].PengTiaoMode_name == FAV_MODE_MULTI)
			{
				return true;
			}
			else
			{
				if (Fav_Cur->favorites_val[i].Func_Hour	||
					Fav_Cur->favorites_val[i].Func_Minute)
				{
					return true;
				}
				else
				{
					continue;
				}
			}
		}
    }
    return false;
}
//覆盖原有收藏夹相同功能
void Favorites_Cover_Func()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
    for (uint8_t i = 0; i < 8; i++)
	{
        if (input_Mode_name == Fav_Cur->favorites_val[i].PengTiaoMode_name)
		{
			if (input_Mode_name == FAV_MODE_MULTI)
			{
				Fun_favorites_Value *item = &Fav_Cur->favorites_val[i];
				item->temperature = 0;
				item->Func_Steam = 0;
				item->PengTiaoMode_name = FAV_MODE_MULTI;
				item->Probe_temp = 0;

				Favorites_Value = Func_SUM_Value_step;
				Fav_Cur->favorites_val[i].PengTiao_Mode = Favorites_Value.Func_Value_step_1.PengTiaoMode_num;
				return;
			}

			// Six 菜单需同时匹配具体菜谱，避免覆盖其他菜谱的收藏位置（与 Favorites_Check_Exists 一致）
			if (input_Mode_name == FAV_MODE_SIX &&
			    Fav_Cur->favorites_val[i].Six_Cook_Fun != input_Six_num)
			{
				continue;
			}
			Fun_favorites_Value *item = &Fav_Cur->favorites_val[i];
			item->temperature = input_Temp;
			item->Func_Steam = input_Is_Steam;
			item->PengTiaoMode_name = input_Mode_name;
			item->Probe_temp = is_probe_inserted() ? input_Temp_probe[0] : 0;	//无探针不存探针温度（probe_target_temp 默认 80，防卡片误显示）
			item->PengTiao_Mode = input_Cooking_Mode;
			item->temp_down = input_Temp_Conventional_Dowm;	//下腔温度随收藏保存（与 Input_favorites 一致）

			if (is_probe_inserted())	//探针模式下不用收藏时间
			{
				item->Func_Hour = 0;
				item->Func_Minute = 0;
			}
			else
			{
				item->Func_Hour = input_Hour;
				item->Func_Minute = input_Minute;
			}
			if (input_Mode_name == FAV_MODE_SIX)
			{
				item->Six_Cook_Fun = input_Six_num;
				item->Six_KaoSe = (int8_t)toastcolor_degree_value();
				item->Six_FaJiao = (g_rising_choice == 1) ? 1 : 0;
				item->Six_KG = (int8_t)toastcolor_weight_value();
				item->Six_Maturity = (int8_t)six_maturity_idx();
			}
			return;
		}
    }
}
//检查收藏夹是否已满，fales：无空位、true：有空位
bool Favorites_Check_Is_full()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
    if (find_lowest_bit_zero_position(Fav_Cur->has_favorites_byte) == -1)
		return false;		// 无空位
	return true;			// 有空位
}

void Fav_Start()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	if (is_door_open()) //启动检测门
	{
		g_send.buzzer_req = BUZZER_KEY_INVALID;
		return;
	}
	favorites_start_selected();	//按收藏类型恢复参数并直接进入运行（nav_favorites.c）
}
//编码器逻辑
void encoder_favorites_action(char key)
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	screen_favorites_t *scr = &g_fav_screen;

	if (!scr->group || !scr->group_sub_1)	//组已删除（防御）：忽略按键，防对 NULL 组移焦点断言闪退
	{
		g_send.buzzer_req = BUZZER_KEY_INVALID;
		return;
	}

	if (favorites_how_many)
	{
		if(key == KEY_ENCODER_CW)          	//编码器顺时针旋转
		{
			if (scr->group == lv_group_get_default())
			{
				favorites_choose_which++;
				if (favorites_choose_which <= favorites_how_many)
				{
					if (favorites_choose_which == 5)
					{
						FAV_screen_Refresh_SecondPage();
					}
					
					lv_group_focus_next(lv_group_get_default());
				}
				else
				{
					favorites_choose_which--;
					g_send.buzzer_req = BUZZER_KEY_INVALID;
				}
			}
			else
			{
				lv_group_focus_next(lv_group_get_default());
			}
			
		}
		else if(key == KEY_ENCODER_CCW)     //编码器逆时针旋转
		{
			if (scr->group == lv_group_get_default())
			{
				favorites_choose_which--;
				if (favorites_choose_which > 0)
				{
					if (favorites_choose_which == 4)
					{
						FAV_screen_Refresh_FirstPage();
					}

					lv_group_focus_prev(lv_group_get_default());
				}
				else
				{
					favorites_choose_which++;
					g_send.buzzer_req = BUZZER_KEY_INVALID;
				}
			}
			else
			{
				lv_group_focus_prev(lv_group_get_default());
			}
		}
		else if(key == KEY_ENCODER_PRESS)         //编码器按下
		{
			if (Del_Fav_create_flag)	//创建删除界面的逻辑
			{
				if (scr->group == lv_group_get_default())
				{
					if (scr->favorites_box_1_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_1);
						lv_group_focus_obj(scr->Favorites_full_delete_1_Btn);
					}
					else if (scr->favorites_box_2_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_2);
						lv_group_focus_obj(scr->Favorites_full_delete_2_Btn);
					}
					else if (scr->favorites_box_3_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_3);
						lv_group_focus_obj(scr->Favorites_full_delete_3_Btn);
					}
					else if (scr->favorites_box_4_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_4);
						lv_group_focus_obj(scr->Favorites_full_delete_4_Btn);
					}
				}
				else if (scr->group_sub_1 == lv_group_get_default())
				{
					Delete_favorites(favorites_choose_which - 1);
					if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
					else FAV_screen_Refresh_SecondPage();
				}
				else if (scr->group_sub_2 == lv_group_get_default())
				{
					Delete_favorites(favorites_choose_which - 1);
					if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
					else FAV_screen_Refresh_SecondPage();
				}
				else if (scr->group_sub_3 == lv_group_get_default())
				{
					Delete_favorites(favorites_choose_which - 1);
					if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
					else FAV_screen_Refresh_SecondPage();
				}
				else if (scr->group_sub_4 == lv_group_get_default())
				{
					Delete_favorites(favorites_choose_which - 1);
					if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
					else FAV_screen_Refresh_SecondPage();
				}
			}
			else
			{
				if (scr->group == lv_group_get_default())
				{
					if (scr->favorites_box_1_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_1);
						lv_group_focus_obj(scr->favo_Start_Btn_1);
					}
					else if (scr->favorites_box_2_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_2);
						lv_group_focus_obj(scr->favo_Start_Btn_2);
					}
					else if (scr->favorites_box_3_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_3);
						lv_group_focus_obj(scr->favo_Start_Btn_3);
					}
					else if (scr->favorites_box_4_Btn == lv_group_get_focused(scr->group))
					{
						lv_group_focus_obj(scr->obj_null);
						lv_group_set_default(scr->group_sub_4);
						lv_group_focus_obj(scr->favo_Start_Btn_4);
					}
				}
				else if (scr->group_sub_1 == lv_group_get_default())
				{
					if (scr->favo_Start_Btn_1 == lv_group_get_focused(scr->group_sub_1))
					{
						Fav_Start();
					}
					else if (scr->favo_Delete_Btn_1 == lv_group_get_focused(scr->group_sub_1))
					{
						Delete_favorites(favorites_choose_which - 1);
						if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
						else FAV_screen_Refresh_SecondPage();
					}
				}
				else if (scr->group_sub_2 == lv_group_get_default())
				{
					if (scr->favo_Start_Btn_2 == lv_group_get_focused(scr->group_sub_2))
					{
						Fav_Start();
					}
					else if (scr->favo_Delete_Btn_2 == lv_group_get_focused(scr->group_sub_2))
					{
						Delete_favorites(favorites_choose_which - 1);
						if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
						else FAV_screen_Refresh_SecondPage();
					}
				}
				else if (scr->group_sub_3 == lv_group_get_default())
				{
					if (scr->favo_Start_Btn_3 == lv_group_get_focused(scr->group_sub_3))
					{
						Fav_Start();
					}
					else if (scr->favo_Delete_Btn_3 == lv_group_get_focused(scr->group_sub_3))
					{
						Delete_favorites(favorites_choose_which - 1);
						if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
						else FAV_screen_Refresh_SecondPage();
					}
				}
				else if (scr->group_sub_4 == lv_group_get_default())
				{
					if (scr->favo_Start_Btn_4 == lv_group_get_focused(scr->group_sub_4))
					{
						Fav_Start();
					}
					else if (scr->favo_Delete_Btn_4 == lv_group_get_focused(scr->group_sub_4))
					{
						Delete_favorites(favorites_choose_which - 1);
						if (favorites_choose_which < 5) FAV_screen_Refresh_FirstPage();
						else FAV_screen_Refresh_SecondPage();
					}
				}
			}
			
			print_status(Fav_Cur);
		}
	}
	else
	{
		g_send.buzzer_req = BUZZER_KEY_INVALID;
	}

}

//返回键逻辑
void return_favorites_action()
{
	screen_favorites_t *scr = &g_fav_screen;

	if (scr->group == lv_group_get_default())
	{
		favo_safety_group_delete();
		Del_Fav_create_flag = 0;
		/* 收藏屏幕是当前活动屏，pop 后由目标页 auto_del=true 加载删除：
		 * 必须置空 obj，否则下次进入收藏页会 lv_obj_del 悬空指针导致冻结 */
		scr->obj = NULL;
		page_pop();		//收藏页返回：nav_pop.c 分支回主菜单
	}
	else
	{
		if (scr->group_sub_1 == lv_group_get_default())
		{
			lv_group_focus_obj(scr->obj_null_1);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_1_Btn);
		}
		else if (scr->group_sub_2 == lv_group_get_default())
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_2_Btn);
		}
		else if(scr->group_sub_3 == lv_group_get_default())
		{
			lv_group_focus_obj(scr->obj_null_3);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_3_Btn);
		}
		else if(scr->group_sub_4 == lv_group_get_default())
		{
			lv_group_focus_obj(scr->obj_null_4);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_4_Btn);
		}
	}
}
//收藏选项标题选择
void FAV_Option_Title(lv_obj_t *obj, const Fun_favorites_Value *fav)
{
	const char *name = NULL;
	if (fav->PengTiaoMode_name == FAV_MODE_SIX)
	{
		/* 临时按收藏菜谱编号取名称（我方表驱动，读后恢复） */
		uint8_t old = g_six_bread_type;
		g_six_bread_type = (uint8_t)fav->Six_Cook_Fun;
		name = six_bread_name();
		g_six_bread_type = old;
	}
	else if (fav->PengTiaoMode_name == FAV_MODE_MULTI)
	{
		name = tr("多段烹饪");
	}
	else
	{
		name = fav_mode_name(fav);
	}
	if (name) lv_label_set_text(obj, name);
	printf("[fav-set] Title obj=%p mode=%d name=%s\n", obj, fav->PengTiaoMode_name, name ? name : "(null)");
}
//多段烹饪，选择功能名称
const char* FAV_Option_Fun_name(int8_t Cooking_FUN)
{
	switch (Cooking_FUN)
    {
        case MODE_UPDOWN_BBQ:     return tr("上下烧烤");
        case MODE_TOP_BBQ:        return tr("顶部烧烤");
        case MODE_BOTTOM_BBQ:     return tr("底部烧烤");
        case MODE_HOT_BBQ:        return tr("热风烧烤");
        case MODE_HOTWIND_BBQ:    return tr("热风");
        case MODE_SAVE_BBQ:       return tr("节能热风");
        case MODE_CENTRAL_BBQ:    return tr("集中烧烤");
        case MODE_WINDCHANGE_BBQ: return tr("热风对流");
        case MODE_AIR:            return tr("空气炸");
        case MODE_SLOWCOOK:       return tr("慢煮");
        case MODE_UNFROZEN:       return tr("解冻");
        case MODE_RISING:         return tr("发酵");
        case MODE_CORN:           return tr("干果");
        case MODE_PIZZA_2:        return tr("披萨");
        case MODE_HEATCONTAIN:    return tr("保温");
        default:                  return tr("无");
    }
}
//标签填写字符串
void FAV_Option_LB_str(lv_obj_t *obj, int8_t Cooking_FUN)
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
    screen_favorites_t *scr = &g_fav_screen;

    if (Cooking_FUN == FAV_MODE_MULTI)
    {
        // 多段烹饪处理
        for (int i = 0; i < 4; i++)
		{
            lv_obj_t *label1 = NULL, *label2 = NULL, *label3 = NULL;

            // 根据行号选择对应的标签对象
            switch(i) {
                case 0:
                    label1 = scr->favo_L1_Lb_1;
                    label2 = scr->favo_L2_Lb_1;
                    label3 = scr->favo_L3_Lb_1;
                    break;
                case 1:
                    label1 = scr->favo_L1_Lb_2;
                    label2 = scr->favo_L2_Lb_2;
                    label3 = scr->favo_L3_Lb_2;
                    break;
                case 2:
                    label1 = scr->favo_L1_Lb_3;
                    label2 = scr->favo_L2_Lb_3;
                    label3 = scr->favo_L3_Lb_3;
                    break;
                case 3:
                    label1 = scr->favo_L1_Lb_4;
                    label2 = scr->favo_L2_Lb_4;
                    label3 = scr->favo_L3_Lb_4;
                    break;
            }

            if (obj == label1) {
                char str[32] = "";
                snprintf(str, sizeof(str), "%s%s", tr("步骤一："), FAV_Option_Fun_name(Favorites_Value.Func_num_1));
                lv_label_set_text(obj, str);
                break;
            }
            else if (obj == label2) {
                char str[32] = "";
                snprintf(str, sizeof(str), "%s%s", tr("步骤二："), FAV_Option_Fun_name(Favorites_Value.Func_num_2));
                lv_label_set_text(obj, str);
                break;
            }
            else if (obj == label3) {
                char str[32] = "";
                snprintf(str, sizeof(str), "%s%s", tr("步骤三："), FAV_Option_Fun_name(Favorites_Value.Func_num_3));
                lv_label_set_text(obj, str);
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < 4; i++)
		{
            lv_obj_t *label1 = NULL, *label2 = NULL, *label3 = NULL;

            // 根据行号选择对应的标签对象
            switch(i) {
                case 0:
                    label1 = scr->favo_L1_Lb_1;
                    label2 = scr->favo_L2_Lb_1;
                    label3 = scr->favo_L3_Lb_1;
                    break;
                case 1:
                    label1 = scr->favo_L1_Lb_2;
                    label2 = scr->favo_L2_Lb_2;
                    label3 = scr->favo_L3_Lb_2;
                    break;
                case 2:
                    label1 = scr->favo_L1_Lb_3;
                    label2 = scr->favo_L2_Lb_3;
                    label3 = scr->favo_L3_Lb_3;
                    break;
                case 3:
                    label1 = scr->favo_L1_Lb_4;
                    label2 = scr->favo_L2_Lb_4;
                    label3 = scr->favo_L3_Lb_4;
                    break;
            }

            if (obj == label1)
			{
				printf("[fav-LB] hit i=%d L1 obj=%p\n", i, obj);
				lv_label_set_text(obj, tr("温度："));
				break;
            }
            else if (obj == label2) {
                printf("[fav-LB] hit i=%d L2 obj=%p h=%d m=%d\n", i, obj,
                       Fav_Cur->favorites_val[i].Func_Hour, Fav_Cur->favorites_val[i].Func_Minute);
                lv_label_set_text_fmt(obj, tr("时间：%d小时%02d分钟"),
                    Fav_Cur->favorites_val[i].Func_Hour,
                    Fav_Cur->favorites_val[i].Func_Minute);
                break;
            }
            else if (obj == label3) {
                if (Cooking_FUN == FAV_MODE_SIX)
                    lv_label_set_text(obj, " ");
                else
                    lv_label_set_text(obj, " ");   /* 本机型无蒸汽辅助：第三行固定留空 */
                break;
            }
        }
    }
}
//收藏选项蒸汽挡位
void FAV_Option_SteamGear(lv_obj_t *obj, int8_t Steam_Gear)
{
	switch (Steam_Gear)
	{
		case 0:
			lv_label_set_text(obj, tr("蒸汽辅助：否"));
			break;
		case 1:
			lv_label_set_text(obj, tr("蒸汽辅助：低"));
			break;
		case 2:
			lv_label_set_text(obj, tr("蒸汽辅助：中"));
			break;
		case 3:
			lv_label_set_text(obj, tr("蒸汽辅助：高"));
			break;

		default:
			break;
	}
}

/**
 * @brief 设置收藏卡片第三行标签
 *        Six菜单：两行小结占用 L1/L2 且菜谱含 "预计烹饪时间：" 时显示时间，否则空格；
 *        其他模式保持原逻辑
 */
static void FAV_Set_L3_Lb(lv_obj_t *lb, const Fun_favorites_Value *fav)
{
	if (fav->PengTiaoMode_name == FAV_MODE_SIX)
	{
		char s1[256], s2[256], tbuf[64];
		int lines, has_cook_time;
		FAV_Six_Parse(fav, s1, sizeof(s1), s2, sizeof(s2), &lines,
		              tbuf, sizeof(tbuf), &has_cook_time);
		if (lines >= 2 && has_cook_time)
			lv_label_set_text(lb, tbuf);
		else
			lv_label_set_text(lb, " ");
	}
	else
		FAV_Option_LB_str(lb, fav->PengTiaoMode_name);
}

//删除选项后实现第一页的页面刷新
void FAV_screen_Refresh_FirstPage()
{
	printf("[fav-refresh] first page\n");
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	screen_favorites_t *scr = &g_fav_screen;

	FAV_Option_Title(scr->favo_Title_Lb_1, &Fav_Cur->favorites_val[0]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_1, &Fav_Cur->favorites_val[0]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_1, &Fav_Cur->favorites_val[0]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_1, &Fav_Cur->favorites_val[0]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_1, &Fav_Cur->favorites_val[0]);

	FAV_Option_Title(scr->favo_Title_Lb_2, &Fav_Cur->favorites_val[1]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_2, &Fav_Cur->favorites_val[1]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_2, &Fav_Cur->favorites_val[1]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_2, &Fav_Cur->favorites_val[1]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_2, &Fav_Cur->favorites_val[1]);

	FAV_Option_Title(scr->favo_Title_Lb_3, &Fav_Cur->favorites_val[2]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_3, &Fav_Cur->favorites_val[2]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_3, &Fav_Cur->favorites_val[2]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_3, &Fav_Cur->favorites_val[2]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_3, &Fav_Cur->favorites_val[2]);

	FAV_Option_Title(scr->favo_Title_Lb_4, &Fav_Cur->favorites_val[3]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_4, &Fav_Cur->favorites_val[3]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_4, &Fav_Cur->favorites_val[3]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_4, &Fav_Cur->favorites_val[3]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_4, &Fav_Cur->favorites_val[3]);

	lv_obj_add_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
	lv_obj_remove_flag(scr->plus_sign_1, LV_OBJ_FLAG_HIDDEN);      	//显示加号+

    if (Fav_Cur->has_favorites_byte & 0b1)
    {
		lv_obj_add_flag(scr->plus_sign_1, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);     	//显示收藏一内容
        lv_group_focus_obj(scr->obj_null_1);
    }
    if (Fav_Cur->has_favorites_byte & 0b10)
    {
		lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);     	//显示收藏二内容
        lv_group_focus_obj(scr->obj_null_2);
    }
    if (Fav_Cur->has_favorites_byte & 0b100)
    {
		lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);     	//显示收藏三内容
        lv_group_focus_obj(scr->obj_null_3);
    }
	if (Fav_Cur->has_favorites_byte & 0b1000)
    {
		lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);     	//显示收藏四内容
        lv_group_focus_obj(scr->obj_null_4);
    }

	//底部页标icon显示逻辑
	lv_obj_add_flag(scr->fav_page_1_No, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->fav_page_2_yes, LV_OBJ_FLAG_HIDDEN);
	lv_obj_remove_flag(scr->fav_page_1_yes, LV_OBJ_FLAG_HIDDEN);
	lv_obj_remove_flag(scr->fav_page_2_No, LV_OBJ_FLAG_HIDDEN);

	if (scr->group_sub_1 == lv_group_get_default())
	{
		if (favorites_how_many >= 1)
		{
			lv_group_focus_obj(scr->obj_null_1);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_1_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_1);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->obj_null);
		}
		
	}
	if (scr->group_sub_2 == lv_group_get_default())
	{
		if (favorites_how_many >= 2)
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_2_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_1_Btn);
			favorites_choose_which--;
		}
	}
	if (scr->group_sub_3 == lv_group_get_default())
	{
		if (favorites_how_many >= 3)
		{
			lv_group_focus_obj(scr->obj_null_3);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_3_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_3);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_2_Btn);
			favorites_choose_which--;
		}
	}
	if (scr->group_sub_4 == lv_group_get_default())
	{
		if (favorites_how_many >= 4)
		{
			lv_group_focus_obj(scr->obj_null_4);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_4_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_3_Btn);
			favorites_choose_which--;
		}
	}
}

//删除选项后实现第二页的页面刷新
void FAV_screen_Refresh_SecondPage()
{
	Fav_Select_By_Probe();	//按当前探针模式选择收藏集合
	screen_favorites_t *scr = &g_fav_screen;

	FAV_Option_Title(scr->favo_Title_Lb_1, &Fav_Cur->favorites_val[4]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_1, &Fav_Cur->favorites_val[4]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_1, &Fav_Cur->favorites_val[4]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_1, &Fav_Cur->favorites_val[4]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_1, &Fav_Cur->favorites_val[4]);

	FAV_Option_Title(scr->favo_Title_Lb_2, &Fav_Cur->favorites_val[5]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_2, &Fav_Cur->favorites_val[5]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_2, &Fav_Cur->favorites_val[5]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_2, &Fav_Cur->favorites_val[5]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_2, &Fav_Cur->favorites_val[5]);

	FAV_Option_Title(scr->favo_Title_Lb_3, &Fav_Cur->favorites_val[6]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_3, &Fav_Cur->favorites_val[6]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_3, &Fav_Cur->favorites_val[6]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_3, &Fav_Cur->favorites_val[6]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_3, &Fav_Cur->favorites_val[6]);

	FAV_Option_Title(scr->favo_Title_Lb_4, &Fav_Cur->favorites_val[7]);
	FAV_Set_L1_Lb(scr->favo_L1_Lb_4, &Fav_Cur->favorites_val[7]);
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_4, &Fav_Cur->favorites_val[7]);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_4, &Fav_Cur->favorites_val[7]);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_4, &Fav_Cur->favorites_val[7]);

	lv_obj_add_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
	lv_obj_remove_flag(scr->plus_sign_1, LV_OBJ_FLAG_HIDDEN);      	//显示加号+

    if (Fav_Cur->has_favorites_byte & 0b10000)
    {
		lv_obj_add_flag(scr->plus_sign_1, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);     	//显示收藏一内容
        lv_group_focus_obj(scr->obj_null_1);
    }
    if (Fav_Cur->has_favorites_byte & 0b100000)
    {
		lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);     	//显示收藏二内容
        lv_group_focus_obj(scr->obj_null_2);
    }
    if (Fav_Cur->has_favorites_byte & 0b1000000)
    {
		lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);     	//显示收藏三内容
        lv_group_focus_obj(scr->obj_null_3);
    }
	if (Fav_Cur->has_favorites_byte & 0b10000000)
    {
		lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);     	//显示收藏四内容
        lv_group_focus_obj(scr->obj_null_4);
    }

	//底部页标icon显示逻辑
	lv_obj_remove_flag(scr->fav_page_1_No, LV_OBJ_FLAG_HIDDEN);
	lv_obj_remove_flag(scr->fav_page_2_yes, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->fav_page_1_yes, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->fav_page_2_No, LV_OBJ_FLAG_HIDDEN);

	if (scr->group_sub_1 == lv_group_get_default())
	{
		if (favorites_how_many >= 5)
		{
			lv_group_focus_obj(scr->obj_null_1);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_1_Btn);
		}
		else
		{
			favorites_choose_which = 4;
			FAV_screen_Refresh_FirstPage();
			lv_group_focus_obj(scr->obj_null_4);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_4_Btn);
		}
		
	}
	if (scr->group_sub_2 == lv_group_get_default())
	{
		if (favorites_how_many >= 6)
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_2_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_1_Btn);
			favorites_choose_which--;
		}
	}
	if (scr->group_sub_3 == lv_group_get_default())
	{
		if (favorites_how_many >= 7)
		{
			lv_group_focus_obj(scr->obj_null_3);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_3_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_3);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_2_Btn);
			favorites_choose_which--;
		}
	}
	if (scr->group_sub_4 == lv_group_get_default())
	{
		if (favorites_how_many >= 8)
		{
			lv_group_focus_obj(scr->obj_null_4);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_4_Btn);
		}
		else
		{
			lv_group_focus_obj(scr->obj_null_2);
			lv_group_set_default(scr->group);
			lv_group_focus_obj(scr->favorites_box_3_Btn);
			favorites_choose_which--;
		}
	}
}


//收藏夹页面创建
void screen_favorites_create(void)
{
	Fav_Select_By_Probe();	//进入收藏界面时读取 Probe_Mode，选择对应的收藏集合
    screen_favorites_t *scr = &g_fav_screen;

    if (scr->obj) {
        /* 对齐同事 auto_del=true 语义：进入收藏页一律重建（同事版
         * if (!ui->auto_del && scr->obj) return 在 auto_del=true 时永不 return）。
         * obj 残留时先安全清理，防止守卫跳过文本设置导致陈旧/空内容错乱 */
        favo_safety_group_delete();
        if (scr->obj == lv_scr_act())   /* 仅活动屏可安全删除；悬空指针只清引用 */
            lv_obj_del(scr->obj);
        scr->obj = NULL;
    }
    printf("[fav-create] probe=%d cur=%s byte=0x%02X\n", is_probe_inserted(),
           Fav_Cur == &Func_favorites_Value_Probe ? "PROBE" : "NORM",
           Fav_Cur->has_favorites_byte);
    for (int k = 0; k < 4; k++)
        printf("  v[%d] mode=%d temp=%d h=%d m=%d probe=%d\n", k,
               Fav_Cur->favorites_val[k].PengTiaoMode_name,
               Fav_Cur->favorites_val[k].temperature,
               Fav_Cur->favorites_val[k].Func_Hour,
               Fav_Cur->favorites_val[k].Func_Minute,
               Fav_Cur->favorites_val[k].Probe_temp);

	// Fav_Cur->has_favorites_byte = 0x7F;

	favorites_choose_which = 1;

    // Init scr->obj
    scr->obj = lv_obj_create(NULL);
    lv_obj_set_scrollbar_mode(scr->obj, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->obj
    lv_obj_set_style_bg_color(scr->obj, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->obj, LVGL_IMAGE_PATH(bg.jpg), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_image_opa(scr->obj, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor(scr->obj, lv_color_hex(0x00ff00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_recolor_opa(scr->obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->fav_title_Lb
    scr->fav_title_Lb = lv_label_create(scr->obj);
    lv_label_set_text(scr->fav_title_Lb, tr("收藏功能"));
    lv_label_set_long_mode(scr->fav_title_Lb, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->fav_title_Lb, 24, 24);
    lv_obj_set_size(scr->fav_title_Lb, 116, 32);

    // Set style of scr->fav_title_Lb
    lv_obj_set_style_text_font(scr->fav_title_Lb, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->fav_title_Lb, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->fav_title_Lb, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->fav_title_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->fav_title_Lb, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favorites_box_1_Btn
    scr->favorites_box_1_Btn = lv_btn_create(scr->obj);
    lv_obj_t *favorites_box_1_Btn_label = lv_label_create(scr->favorites_box_1_Btn);
    lv_label_set_text(favorites_box_1_Btn_label, "");
    lv_obj_align(favorites_box_1_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favorites_box_1_Btn, 22, 80);
    lv_obj_set_size(scr->favorites_box_1_Btn, 303, 377);

    // Set style of scr->favorites_box_1_Btn
    lv_obj_set_style_bg_color(scr->favorites_box_1_Btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favorites_box_1_Btn, LVGL_IMAGE_PATH(favorites_select_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favorites_box_1_Btn, &c_montserratmedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favorites_box_1_Btn, LVGL_IMAGE_PATH(favorites_select_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(scr->favorites_box_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favorites_box_2_Btn
    scr->favorites_box_2_Btn = lv_btn_create(scr->obj);
    lv_obj_t *favorites_box_2_Btn_label = lv_label_create(scr->favorites_box_2_Btn);
    lv_label_set_text(favorites_box_2_Btn_label, "");
    lv_obj_align(favorites_box_2_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favorites_box_2_Btn, 333, 80);
    lv_obj_set_size(scr->favorites_box_2_Btn, 303, 377);

    // Set style of scr->favorites_box_2_Btn
    lv_obj_set_style_bg_color(scr->favorites_box_2_Btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favorites_box_2_Btn, LVGL_IMAGE_PATH(favorites_select_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favorites_box_2_Btn, &c_montserratmedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favorites_box_2_Btn, LVGL_IMAGE_PATH(favorites_select_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(scr->favorites_box_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favorites_box_3_Btn
    scr->favorites_box_3_Btn = lv_btn_create(scr->obj);
    lv_obj_t *favorites_box_3_Btn_label = lv_label_create(scr->favorites_box_3_Btn);
    lv_label_set_text(favorites_box_3_Btn_label, "");
    lv_obj_align(favorites_box_3_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favorites_box_3_Btn, 643, 80);
    lv_obj_set_size(scr->favorites_box_3_Btn, 303, 377);

    // Set style of scr->favorites_box_3_Btn
    lv_obj_set_style_bg_color(scr->favorites_box_3_Btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favorites_box_3_Btn, LVGL_IMAGE_PATH(favorites_select_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favorites_box_3_Btn, &c_montserratmedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favorites_box_3_Btn, LVGL_IMAGE_PATH(favorites_select_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(scr->favorites_box_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favorites_box_4_Btn
    scr->favorites_box_4_Btn = lv_btn_create(scr->obj);
    lv_obj_t *favorites_box_4_Btn_label = lv_label_create(scr->favorites_box_4_Btn);
    lv_label_set_text(favorites_box_4_Btn_label, "");
    lv_obj_align(favorites_box_4_Btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favorites_box_4_Btn, 954, 80);
    lv_obj_set_size(scr->favorites_box_4_Btn, 303, 377);

    // Set style of scr->favorites_box_4_Btn
    lv_obj_set_style_bg_color(scr->favorites_box_4_Btn, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favorites_box_4_Btn, LVGL_IMAGE_PATH(favorites_select_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favorites_box_4_Btn, &c_montserratmedium_16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favorites_box_4_Btn, LVGL_IMAGE_PATH(favorites_select_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_ofs_y(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_width(scr->favorites_box_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->fav_page_1_No
    scr->fav_page_1_No = lv_img_create(scr->obj);
    lv_img_set_src(scr->fav_page_1_No, LVGL_IMAGE_PATH(fav_page_sign_1.png));
    lv_img_set_pivot(scr->fav_page_1_No, 50, 50);
    lv_img_set_angle(scr->fav_page_1_No, 0);
    lv_obj_set_style_img_opa(scr->fav_page_1_No, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->fav_page_1_No, 615, 464);
    lv_obj_add_flag(scr->fav_page_1_No, LV_OBJ_FLAG_HIDDEN);

    // Init scr->fav_page_2_No
    scr->fav_page_2_No = lv_img_create(scr->obj);
    lv_img_set_src(scr->fav_page_2_No, LVGL_IMAGE_PATH(fav_page_sign_1.png));
    lv_img_set_pivot(scr->fav_page_2_No, 50, 50);
    lv_img_set_angle(scr->fav_page_2_No, 0);
    lv_obj_set_style_img_opa(scr->fav_page_2_No, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->fav_page_2_No, 655, 464);

    // Init scr->fav_page_1_yes
    scr->fav_page_1_yes = lv_img_create(scr->obj);
    lv_img_set_src(scr->fav_page_1_yes, LVGL_IMAGE_PATH(fav_page_sign_2.png));
    lv_img_set_pivot(scr->fav_page_1_yes, 50, 50);
    lv_img_set_angle(scr->fav_page_1_yes, 0);
    lv_obj_set_style_img_opa(scr->fav_page_1_yes, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->fav_page_1_yes, 616, 465);

    // Init scr->fav_page_2_yes
    scr->fav_page_2_yes = lv_img_create(scr->obj);
    lv_img_set_src(scr->fav_page_2_yes, LVGL_IMAGE_PATH(fav_page_sign_2.png));
    lv_img_set_pivot(scr->fav_page_2_yes, 50, 50);
    lv_img_set_angle(scr->fav_page_2_yes, 0);
    lv_obj_set_style_img_opa(scr->fav_page_2_yes, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->fav_page_2_yes, 656, 465);
    lv_obj_add_flag(scr->fav_page_2_yes, LV_OBJ_FLAG_HIDDEN);

    // Init scr->plus_sign_1
    scr->plus_sign_1 = lv_img_create(scr->obj);
    lv_img_set_src(scr->plus_sign_1, LVGL_IMAGE_PATH(favorites_plus.png));
    lv_img_set_pivot(scr->plus_sign_1, 50, 50);
    lv_img_set_angle(scr->plus_sign_1, 0);
    lv_obj_set_style_img_opa(scr->plus_sign_1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->plus_sign_1, 109, 204);

    // Init scr->plus_sign_2
    scr->plus_sign_2 = lv_img_create(scr->obj);
    lv_img_set_src(scr->plus_sign_2, LVGL_IMAGE_PATH(favorites_plus.png));
    lv_img_set_pivot(scr->plus_sign_2, 50, 50);
    lv_img_set_angle(scr->plus_sign_2, 0);
    lv_obj_set_style_img_opa(scr->plus_sign_2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->plus_sign_2, 420, 204);

    // Init scr->plus_sign_3
    scr->plus_sign_3 = lv_img_create(scr->obj);
    lv_img_set_src(scr->plus_sign_3, LVGL_IMAGE_PATH(favorites_plus.png));
    lv_img_set_pivot(scr->plus_sign_3, 50, 50);
    lv_img_set_angle(scr->plus_sign_3, 0);
    lv_obj_set_style_img_opa(scr->plus_sign_3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->plus_sign_3, 730, 204);

    // Init scr->plus_sign_4
    scr->plus_sign_4 = lv_img_create(scr->obj);
    lv_img_set_src(scr->plus_sign_4, LVGL_IMAGE_PATH(favorites_plus.png));
    lv_img_set_pivot(scr->plus_sign_4, 50, 50);
    lv_img_set_angle(scr->plus_sign_4, 0);
    lv_obj_set_style_img_opa(scr->plus_sign_4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_pos(scr->plus_sign_4, 1041, 204);

    // Init scr->favo_Cont_1
    scr->favo_Cont_1 = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->favo_Cont_1, 23, 76);
    lv_obj_set_size(scr->favo_Cont_1, 300, 382);
    lv_obj_set_scrollbar_mode(scr->favo_Cont_1, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_Cont_1
    lv_obj_set_style_bg_color(scr->favo_Cont_1, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_LB_Cont_1
    scr->favo_LB_Cont_1 = lv_obj_create(scr->favo_Cont_1);
    lv_obj_set_pos(scr->favo_LB_Cont_1, 0, 2);
    lv_obj_set_size(scr->favo_LB_Cont_1, 283, 262);
    lv_obj_set_scrollbar_mode(scr->favo_LB_Cont_1, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_LB_Cont_1
    lv_obj_set_style_bg_color(scr->favo_LB_Cont_1, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_LB_Cont_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Title_Lb_1
    scr->favo_Title_Lb_1 = lv_label_create(scr->favo_LB_Cont_1);
    //lv_label_set_text(scr->favo_Title_Lb_1, TR(STR_Steam_Plus));
	FAV_Option_Title(scr->favo_Title_Lb_1, &Fav_Cur->favorites_val[0]);
	
    lv_label_set_long_mode(scr->favo_Title_Lb_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_Title_Lb_1, 50, 25);
    lv_obj_set_size(scr->favo_Title_Lb_1, 200, 32);

    // Set style of scr->favo_Title_Lb_1
    lv_obj_set_style_text_font(scr->favo_Title_Lb_1, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Title_Lb_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Title_Lb_1, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Title_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Title_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L1_Lb_1
    scr->favo_L1_Lb_1 = lv_label_create(scr->favo_LB_Cont_1);
    //lv_label_set_text(scr->favo_L1_Lb_1, "温度：");
	FAV_Set_L1_Lb(scr->favo_L1_Lb_1, &Fav_Cur->favorites_val[0]);
    lv_label_set_long_mode(scr->favo_L1_Lb_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L1_Lb_1, 23, 94);
    lv_obj_set_size(scr->favo_L1_Lb_1, 250, 35);

    // Set style of scr->favo_L1_Lb_1
    lv_obj_set_style_text_font(scr->favo_L1_Lb_1, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L1_Lb_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L1_Lb_1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L1_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L1_Lb_1, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L1_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Func_Temp_Lb_1
    scr->favo_Func_Temp_Lb_1 = lv_label_create(scr->favo_LB_Cont_1);
    //lv_label_set_text(scr->favo_Func_Temp_Lb_1, "110℃");
	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_1, &Fav_Cur->favorites_val[0]);
    lv_label_set_long_mode(scr->favo_Func_Temp_Lb_1, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(scr->favo_Func_Temp_Lb_1, 101, 93);
    lv_obj_set_size(scr->favo_Func_Temp_Lb_1, 158, 32);

    // Set style of scr->favo_Func_Temp_Lb_1
    lv_obj_set_style_text_font(scr->favo_Func_Temp_Lb_1, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Func_Temp_Lb_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Func_Temp_Lb_1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Func_Temp_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Func_Temp_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L2_Lb_1
    scr->favo_L2_Lb_1 = lv_label_create(scr->favo_LB_Cont_1);
    //lv_label_set_text(scr->favo_L2_Lb_1, "时间： 20分钟");
	// lv_label_set_text_fmt(scr->favo_L2_Lb_1, "时间： %d小时%d分钟", 
	// 	Fav_Cur->favorites_val[0].Func_Hour, Fav_Cur->favorites_val[0].Func_Minute);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_1, &Fav_Cur->favorites_val[0]);
    lv_label_set_long_mode(scr->favo_L2_Lb_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L2_Lb_1, 23, 156);
    lv_obj_set_size(scr->favo_L2_Lb_1, 250, 35);

    // Set style of scr->favo_L2_Lb_1
    lv_obj_set_style_text_font(scr->favo_L2_Lb_1, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L2_Lb_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L2_Lb_1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L2_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L2_Lb_1, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L2_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L3_Lb_1
    scr->favo_L3_Lb_1 = lv_label_create(scr->favo_LB_Cont_1);
    //lv_label_set_text(scr->favo_L3_Lb_1, "蒸汽辅助：否");
	// FAV_Option_SteamGear(scr->favo_L3_Lb_1, Fav_Cur->favorites_val[0].Func_Steam);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_1, &Fav_Cur->favorites_val[0]);
    lv_label_set_long_mode(scr->favo_L3_Lb_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L3_Lb_1, 23, 220);
    lv_obj_set_size(scr->favo_L3_Lb_1, 210, 35);

    // Set style of scr->favo_L3_Lb_1
    lv_obj_set_style_text_font(scr->favo_L3_Lb_1, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L3_Lb_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L3_Lb_1, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L3_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L3_Lb_1, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L3_Lb_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Start_Btn_1
    scr->favo_Start_Btn_1 = lv_btn_create(scr->favo_Cont_1);
    lv_obj_t *favo_Start_Btn_1_label = lv_label_create(scr->favo_Start_Btn_1);
    lv_label_set_text(favo_Start_Btn_1_label, tr("启动"));
    lv_obj_align(favo_Start_Btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Start_Btn_1, 22, 302);
    lv_obj_set_size(scr->favo_Start_Btn_1, 84, 52);

    // Set style of scr->favo_Start_Btn_1
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_1, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Start_Btn_1, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_1, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Delete_Btn_1
    scr->favo_Delete_Btn_1 = lv_btn_create(scr->favo_Cont_1);
    lv_obj_t *favo_Delete_Btn_1_label = lv_label_create(scr->favo_Delete_Btn_1);
    lv_label_set_text(favo_Delete_Btn_1_label, tr("删除"));
    lv_obj_align(favo_Delete_Btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Delete_Btn_1, 195, 302);
    lv_obj_set_size(scr->favo_Delete_Btn_1, 84, 52);

    // Set style of scr->favo_Delete_Btn_1
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_1, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Delete_Btn_1, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_1, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_1, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Cont_2
    scr->favo_Cont_2 = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->favo_Cont_2, 333, 76);
    lv_obj_set_size(scr->favo_Cont_2, 300, 382);
    lv_obj_set_scrollbar_mode(scr->favo_Cont_2, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_Cont_2
    lv_obj_set_style_bg_color(scr->favo_Cont_2, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_LB_Cont_2
    scr->favo_LB_Cont_2 = lv_obj_create(scr->favo_Cont_2);
    lv_obj_set_pos(scr->favo_LB_Cont_2, 0, 2);
    lv_obj_set_size(scr->favo_LB_Cont_2, 283, 262);
    lv_obj_set_scrollbar_mode(scr->favo_LB_Cont_2, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_LB_Cont_2
    lv_obj_set_style_bg_color(scr->favo_LB_Cont_2, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_LB_Cont_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Title_Lb_2
    scr->favo_Title_Lb_2 = lv_label_create(scr->favo_LB_Cont_2);
    //lv_label_set_text(scr->favo_Title_Lb_2, TR(STR_Steam_Plus));
	FAV_Option_Title(scr->favo_Title_Lb_2, &Fav_Cur->favorites_val[1]);
    lv_label_set_long_mode(scr->favo_Title_Lb_2, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(scr->favo_Title_Lb_2, 50, 25);
    lv_obj_set_size(scr->favo_Title_Lb_2, 200, 32);

    // Set style of scr->favo_Title_Lb_2
    lv_obj_set_style_text_font(scr->favo_Title_Lb_2, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Title_Lb_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Title_Lb_2, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Title_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Title_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L1_Lb_2
    scr->favo_L1_Lb_2 = lv_label_create(scr->favo_LB_Cont_2);
    //lv_label_set_text(scr->favo_L1_Lb_2, "发酵阶段：是");
	//lv_label_set_text(scr->favo_L1_Lb_2, "温度：");
	FAV_Set_L1_Lb(scr->favo_L1_Lb_2, &Fav_Cur->favorites_val[1]);
    lv_label_set_long_mode(scr->favo_L1_Lb_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L1_Lb_2, 23, 94);
    lv_obj_set_size(scr->favo_L1_Lb_2, 250, 35);

    // Set style of scr->favo_L1_Lb_2
    lv_obj_set_style_text_font(scr->favo_L1_Lb_2, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L1_Lb_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L1_Lb_2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L1_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L1_Lb_2, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L1_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Func_Temp_Lb_2
    scr->favo_Func_Temp_Lb_2 = lv_label_create(scr->favo_LB_Cont_2);
    //lv_label_set_text(scr->favo_Func_Temp_Lb_2, " ");

	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_2, &Fav_Cur->favorites_val[1]);
    lv_label_set_long_mode(scr->favo_Func_Temp_Lb_2, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(scr->favo_Func_Temp_Lb_2, 101, 93);
    lv_obj_set_size(scr->favo_Func_Temp_Lb_2, 158, 32);

    // Set style of scr->favo_Func_Temp_Lb_2
    lv_obj_set_style_text_font(scr->favo_Func_Temp_Lb_2, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Func_Temp_Lb_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Func_Temp_Lb_2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Func_Temp_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Func_Temp_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L2_Lb_2
    scr->favo_L2_Lb_2 = lv_label_create(scr->favo_LB_Cont_2);
    //lv_label_set_text(scr->favo_L2_Lb_2, "时间：24分钟");
	// lv_label_set_text_fmt(scr->favo_L2_Lb_2, "时间： %d小时%d分钟", 
	// 	Fav_Cur->favorites_val[1].Func_Hour, Fav_Cur->favorites_val[1].Func_Minute);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_2, &Fav_Cur->favorites_val[1]);
    lv_label_set_long_mode(scr->favo_L2_Lb_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L2_Lb_2, 23, 156);
    lv_obj_set_size(scr->favo_L2_Lb_2, 250, 35);

    // Set style of scr->favo_L2_Lb_2
    lv_obj_set_style_text_font(scr->favo_L2_Lb_2, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L2_Lb_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L2_Lb_2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L2_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L2_Lb_2, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L2_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L3_Lb_2
    scr->favo_L3_Lb_2 = lv_label_create(scr->favo_LB_Cont_2);
    //lv_label_set_text(scr->favo_L3_Lb_2, " ");
	// FAV_Option_SteamGear(scr->favo_L3_Lb_2, Fav_Cur->favorites_val[1].Func_Steam);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_2, &Fav_Cur->favorites_val[1]);
    lv_label_set_long_mode(scr->favo_L3_Lb_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L3_Lb_2, 23, 220);
    lv_obj_set_size(scr->favo_L3_Lb_2, 210, 35);

    // Set style of scr->favo_L3_Lb_2
    lv_obj_set_style_text_font(scr->favo_L3_Lb_2, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L3_Lb_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L3_Lb_2, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L3_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L3_Lb_2, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L3_Lb_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Start_Btn_2
    scr->favo_Start_Btn_2 = lv_btn_create(scr->favo_Cont_2);
    lv_obj_t *favo_Start_Btn_2_label = lv_label_create(scr->favo_Start_Btn_2);
    lv_label_set_text(favo_Start_Btn_2_label, tr("启动"));
    lv_obj_align(favo_Start_Btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Start_Btn_2, 22, 302);
    lv_obj_set_size(scr->favo_Start_Btn_2, 84, 52);

    // Set style of scr->favo_Start_Btn_2
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_2, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Start_Btn_2, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_2, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Delete_Btn_2
    scr->favo_Delete_Btn_2 = lv_btn_create(scr->favo_Cont_2);
    lv_obj_t *favo_Delete_Btn_2_label = lv_label_create(scr->favo_Delete_Btn_2);
    lv_label_set_text(favo_Delete_Btn_2_label, tr("删除"));
    lv_obj_align(favo_Delete_Btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Delete_Btn_2, 195, 302);
    lv_obj_set_size(scr->favo_Delete_Btn_2, 84, 52);

    // Set style of scr->favo_Delete_Btn_2
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_2, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Delete_Btn_2, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_2, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_2, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Cont_3
    scr->favo_Cont_3 = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->favo_Cont_3, 643, 76);
    lv_obj_set_size(scr->favo_Cont_3, 300, 382);
    lv_obj_set_scrollbar_mode(scr->favo_Cont_3, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_Cont_3
    lv_obj_set_style_bg_color(scr->favo_Cont_3, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_LB_Cont_3
    scr->favo_LB_Cont_3 = lv_obj_create(scr->favo_Cont_3);
    lv_obj_set_pos(scr->favo_LB_Cont_3, 0, 2);
    lv_obj_set_size(scr->favo_LB_Cont_3, 283, 262);
    lv_obj_set_scrollbar_mode(scr->favo_LB_Cont_3, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_LB_Cont_3
    lv_obj_set_style_bg_color(scr->favo_LB_Cont_3, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_LB_Cont_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Title_Lb_3
    scr->favo_Title_Lb_3 = lv_label_create(scr->favo_LB_Cont_3);
    //lv_label_set_text(scr->favo_Title_Lb_3, TR(STR_Steam_Plus));
	FAV_Option_Title(scr->favo_Title_Lb_3, &Fav_Cur->favorites_val[2]);
	
    lv_label_set_long_mode(scr->favo_Title_Lb_3, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(scr->favo_Title_Lb_3, 50, 25);
    lv_obj_set_size(scr->favo_Title_Lb_3, 200, 32);

    // Set style of scr->favo_Title_Lb_3
    lv_obj_set_style_text_font(scr->favo_Title_Lb_3, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Title_Lb_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Title_Lb_3, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Title_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Title_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L1_Lb_3
    scr->favo_L1_Lb_3 = lv_label_create(scr->favo_LB_Cont_3);
    //lv_label_set_text(scr->favo_L1_Lb_3, "温度：");
	FAV_Set_L1_Lb(scr->favo_L1_Lb_3, &Fav_Cur->favorites_val[2]);
    lv_label_set_long_mode(scr->favo_L1_Lb_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L1_Lb_3, 23, 94);
    lv_obj_set_size(scr->favo_L1_Lb_3, 250, 35);

    // Set style of scr->favo_L1_Lb_3
    lv_obj_set_style_text_font(scr->favo_L1_Lb_3, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L1_Lb_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L1_Lb_3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L1_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L1_Lb_3, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L1_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Func_Temp_Lb_3
    scr->favo_Func_Temp_Lb_3 = lv_label_create(scr->favo_LB_Cont_3);
    //lv_label_set_text(scr->favo_Func_Temp_Lb_3, "110℃");

	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_3, &Fav_Cur->favorites_val[2]);
    lv_label_set_long_mode(scr->favo_Func_Temp_Lb_3, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(scr->favo_Func_Temp_Lb_3, 101, 93);
    lv_obj_set_size(scr->favo_Func_Temp_Lb_3, 158, 32);

    // Set style of scr->favo_Func_Temp_Lb_3
    lv_obj_set_style_text_font(scr->favo_Func_Temp_Lb_3, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Func_Temp_Lb_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Func_Temp_Lb_3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Func_Temp_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Func_Temp_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L2_Lb_3
    scr->favo_L2_Lb_3 = lv_label_create(scr->favo_LB_Cont_3);
    //lv_label_set_text(scr->favo_L2_Lb_3, "时间： 20分钟");
	// lv_label_set_text_fmt(scr->favo_L2_Lb_3, "时间： %d小时%d分钟", 
	// 	Fav_Cur->favorites_val[2].Func_Hour, Fav_Cur->favorites_val[2].Func_Minute);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_3, &Fav_Cur->favorites_val[2]);
    lv_label_set_long_mode(scr->favo_L2_Lb_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L2_Lb_3, 23, 156);
    lv_obj_set_size(scr->favo_L2_Lb_3, 250, 35);

    // Set style of scr->favo_L2_Lb_3
    lv_obj_set_style_text_font(scr->favo_L2_Lb_3, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L2_Lb_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L2_Lb_3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L2_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L2_Lb_3, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L2_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L3_Lb_3
    scr->favo_L3_Lb_3 = lv_label_create(scr->favo_LB_Cont_3);
    //lv_label_set_text(scr->favo_L3_Lb_3, "蒸汽辅助：否");
	// FAV_Option_SteamGear(scr->favo_L3_Lb_3, Fav_Cur->favorites_val[2].Func_Steam);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_3, &Fav_Cur->favorites_val[2]);
    lv_label_set_long_mode(scr->favo_L3_Lb_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L3_Lb_3, 23, 220);
    lv_obj_set_size(scr->favo_L3_Lb_3, 210, 35);

    // Set style of scr->favo_L3_Lb_3
    lv_obj_set_style_text_font(scr->favo_L3_Lb_3, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L3_Lb_3, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L3_Lb_3, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L3_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L3_Lb_3, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L3_Lb_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Start_Btn_3
    scr->favo_Start_Btn_3 = lv_btn_create(scr->favo_Cont_3);
    lv_obj_t *favo_Start_Btn_3_label = lv_label_create(scr->favo_Start_Btn_3);
    lv_label_set_text(favo_Start_Btn_3_label, tr("启动"));
    lv_obj_align(favo_Start_Btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Start_Btn_3, 22, 302);
    lv_obj_set_size(scr->favo_Start_Btn_3, 84, 52);

    // Set style of scr->favo_Start_Btn_3
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_3, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Start_Btn_3, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_3, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Delete_Btn_3
    scr->favo_Delete_Btn_3 = lv_btn_create(scr->favo_Cont_3);
    lv_obj_t *favo_Delete_Btn_3_label = lv_label_create(scr->favo_Delete_Btn_3);
    lv_label_set_text(favo_Delete_Btn_3_label, tr("删除"));
    lv_obj_align(favo_Delete_Btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Delete_Btn_3, 195, 302);
    lv_obj_set_size(scr->favo_Delete_Btn_3, 84, 52);

    // Set style of scr->favo_Delete_Btn_3
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_3, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Delete_Btn_3, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_3, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_3, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Cont_4
    scr->favo_Cont_4 = lv_obj_create(scr->obj);
    lv_obj_set_pos(scr->favo_Cont_4, 953, 76);
    lv_obj_set_size(scr->favo_Cont_4, 300, 382);
    lv_obj_set_scrollbar_mode(scr->favo_Cont_4, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_Cont_4
    lv_obj_set_style_bg_color(scr->favo_Cont_4, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_LB_Cont_4
    scr->favo_LB_Cont_4 = lv_obj_create(scr->favo_Cont_4);
    lv_obj_set_pos(scr->favo_LB_Cont_4, 0, 2);
    lv_obj_set_size(scr->favo_LB_Cont_4, 283, 262);
    lv_obj_set_scrollbar_mode(scr->favo_LB_Cont_4, LV_SCROLLBAR_MODE_OFF);

    // Set style of scr->favo_LB_Cont_4
    lv_obj_set_style_bg_color(scr->favo_LB_Cont_4, lv_color_hex(0xeff8f5), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_LB_Cont_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Title_Lb_4
    scr->favo_Title_Lb_4 = lv_label_create(scr->favo_LB_Cont_4);
    //lv_label_set_text(scr->favo_Title_Lb_4, TR(STR_Steam_Plus));
	FAV_Option_Title(scr->favo_Title_Lb_4, &Fav_Cur->favorites_val[3]);
    lv_label_set_long_mode(scr->favo_Title_Lb_4, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(scr->favo_Title_Lb_4, 50, 25);
    lv_obj_set_size(scr->favo_Title_Lb_4, 200, 32);

    // Set style of scr->favo_Title_Lb_4
    lv_obj_set_style_text_font(scr->favo_Title_Lb_4, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Title_Lb_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Title_Lb_4, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Title_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Title_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L1_Lb_4
    scr->favo_L1_Lb_4 = lv_label_create(scr->favo_LB_Cont_4);
    // lv_label_set_text(scr->favo_L1_Lb_4, "温度：");
	FAV_Set_L1_Lb(scr->favo_L1_Lb_4, &Fav_Cur->favorites_val[3]);
    lv_label_set_long_mode(scr->favo_L1_Lb_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L1_Lb_4, 23, 94);
    lv_obj_set_size(scr->favo_L1_Lb_4, 250, 35);

    // Set style of scr->favo_L1_Lb_4
    lv_obj_set_style_text_font(scr->favo_L1_Lb_4, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L1_Lb_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L1_Lb_4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L1_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L1_Lb_4, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L1_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Func_Temp_Lb_4
    scr->favo_Func_Temp_Lb_4 = lv_label_create(scr->favo_LB_Cont_4);
    //lv_label_set_text(scr->favo_Func_Temp_Lb_4, "↑180℃/↓120℃");
	// lv_label_set_text_fmt(scr->favo_Func_Temp_Lb_4, "%d℃", Fav_Cur->favorites_val[3].temperature);

	FAV_Set_FuncTemp_Lb(scr->favo_Func_Temp_Lb_4, &Fav_Cur->favorites_val[3]);

    lv_label_set_long_mode(scr->favo_Func_Temp_Lb_4, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(scr->favo_Func_Temp_Lb_4, 101, 93);
    lv_obj_set_size(scr->favo_Func_Temp_Lb_4, 158, 32);

    // Set style of scr->favo_Func_Temp_Lb_4
    lv_obj_set_style_text_font(scr->favo_Func_Temp_Lb_4, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_Func_Temp_Lb_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_Func_Temp_Lb_4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Func_Temp_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Func_Temp_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L2_Lb_4
    scr->favo_L2_Lb_4 = lv_label_create(scr->favo_LB_Cont_4);
    //lv_label_set_text(scr->favo_L2_Lb_4, "时间： 20分钟");
	// lv_label_set_text_fmt(scr->favo_L2_Lb_4, "时间： %d小时%d分钟", 
	// 	Fav_Cur->favorites_val[3].Func_Hour, Fav_Cur->favorites_val[3].Func_Minute);
	FAV_Set_L2_Lb(scr->favo_L2_Lb_4, &Fav_Cur->favorites_val[3]);
    lv_label_set_long_mode(scr->favo_L2_Lb_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L2_Lb_4, 23, 156);
    lv_obj_set_size(scr->favo_L2_Lb_4, 250, 35);

    // Set style of scr->favo_L2_Lb_4
    lv_obj_set_style_text_font(scr->favo_L2_Lb_4, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L2_Lb_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L2_Lb_4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L2_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L2_Lb_4, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L2_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_L3_Lb_4
    scr->favo_L3_Lb_4 = lv_label_create(scr->favo_LB_Cont_4);
    //lv_label_set_text(scr->favo_L3_Lb_4, "蒸汽辅助：否");
	// FAV_Option_SteamGear(scr->favo_L3_Lb_4, Fav_Cur->favorites_val[3].Func_Steam);
	FAV_Set_L3_Lb(scr->favo_L3_Lb_4, &Fav_Cur->favorites_val[3]);
    lv_label_set_long_mode(scr->favo_L3_Lb_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(scr->favo_L3_Lb_4, 23, 220);
    lv_obj_set_size(scr->favo_L3_Lb_4, 210, 35);

    // Set style of scr->favo_L3_Lb_4
    lv_obj_set_style_text_font(scr->favo_L3_Lb_4, &c_taiwanpearl_regular_24, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(scr->favo_L3_Lb_4, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(scr->favo_L3_Lb_4, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_L3_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(scr->favo_L3_Lb_4, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_L3_Lb_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Init scr->favo_Start_Btn_4
    scr->favo_Start_Btn_4 = lv_btn_create(scr->favo_Cont_4);
    lv_obj_t *favo_Start_Btn_4_label = lv_label_create(scr->favo_Start_Btn_4);
    lv_label_set_text(favo_Start_Btn_4_label, tr("启动"));
    lv_obj_align(favo_Start_Btn_4_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Start_Btn_4, 22, 302);
    lv_obj_set_size(scr->favo_Start_Btn_4, 84, 52);

    // Set style of scr->favo_Start_Btn_4
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_4, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Start_Btn_4, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Start_Btn_4, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Start_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

    // Init scr->favo_Delete_Btn_4
    scr->favo_Delete_Btn_4 = lv_btn_create(scr->favo_Cont_4);
    lv_obj_t *favo_Delete_Btn_4_label = lv_label_create(scr->favo_Delete_Btn_4);
    lv_label_set_text(favo_Delete_Btn_4_label, tr("删除"));
    lv_obj_align(favo_Delete_Btn_4_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_pos(scr->favo_Delete_Btn_4, 195, 302);
    lv_obj_set_size(scr->favo_Delete_Btn_4, 84, 52);

    // Set style of scr->favo_Delete_Btn_4
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_4, LVGL_IMAGE_PATH(fav_btn_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(scr->favo_Delete_Btn_4, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_img_src(scr->favo_Delete_Btn_4, LVGL_IMAGE_PATH(fav_btn_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_border_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_shadow_opa(scr->favo_Delete_Btn_4, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

	//收藏夹满了创建删除键
	if (Del_Fav_create_flag)	//创建删除界面
	{
		lv_obj_add_flag(scr->favo_Start_Btn_1, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Delete_Btn_1, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Start_Btn_2, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Delete_Btn_2, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Start_Btn_3, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Delete_Btn_3, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Start_Btn_4, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->favo_Delete_Btn_4, LV_OBJ_FLAG_HIDDEN);


		// Init scr->Favorites_full_delete_1_Btn
		scr->Favorites_full_delete_1_Btn = lv_btn_create(scr->favo_Cont_1);
		lv_obj_t *Favorites_full_delete_1_Btn_label = lv_label_create(scr->Favorites_full_delete_1_Btn);
		lv_label_set_text(Favorites_full_delete_1_Btn_label, tr("删除"));
		lv_obj_align(Favorites_full_delete_1_Btn_label, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_pos(scr->Favorites_full_delete_1_Btn, 22, 302);
		lv_obj_set_size(scr->Favorites_full_delete_1_Btn, 260, 52);

		// Set style of scr->Favorites_full_delete_1_Btn
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_1_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_font(scr->Favorites_full_delete_1_Btn, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(scr->Favorites_full_delete_1_Btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_width(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_1_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_ofs_y(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_1_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

		// Init scr->Favorites_full_delete_2_Btn
		scr->Favorites_full_delete_2_Btn = lv_btn_create(scr->favo_Cont_2);
		lv_obj_t *Favorites_full_delete_2_Btn_label = lv_label_create(scr->Favorites_full_delete_2_Btn);
		lv_label_set_text(Favorites_full_delete_2_Btn_label, tr("删除"));
		lv_obj_align(Favorites_full_delete_2_Btn_label, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_pos(scr->Favorites_full_delete_2_Btn, 22, 302);
		lv_obj_set_size(scr->Favorites_full_delete_2_Btn, 260, 52);

		// Set style of scr->Favorites_full_delete_2_Btn
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_2_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_font(scr->Favorites_full_delete_2_Btn, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(scr->Favorites_full_delete_2_Btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_width(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_2_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_ofs_y(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_2_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

		// Init scr->Favorites_full_delete_3_Btn
		scr->Favorites_full_delete_3_Btn = lv_btn_create(scr->favo_Cont_3);
		lv_obj_t *Favorites_full_delete_3_Btn_label = lv_label_create(scr->Favorites_full_delete_3_Btn);
		lv_label_set_text(Favorites_full_delete_3_Btn_label, tr("删除"));
		lv_obj_align(Favorites_full_delete_3_Btn_label, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_pos(scr->Favorites_full_delete_3_Btn, 22, 302);
		lv_obj_set_size(scr->Favorites_full_delete_3_Btn, 260, 52);

		// Set style of scr->Favorites_full_delete_3_Btn
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_3_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_font(scr->Favorites_full_delete_3_Btn, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(scr->Favorites_full_delete_3_Btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_width(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_3_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_ofs_y(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_3_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);

		// Init scr->Favorites_full_delete_4_Btn
		scr->Favorites_full_delete_4_Btn = lv_btn_create(scr->favo_Cont_4);
		lv_obj_t *Favorites_full_delete_4_Btn_label = lv_label_create(scr->Favorites_full_delete_4_Btn);
		lv_label_set_text(Favorites_full_delete_4_Btn_label, tr("删除"));
		lv_obj_align(Favorites_full_delete_4_Btn_label, LV_ALIGN_CENTER, 0, 0);
		lv_obj_set_pos(scr->Favorites_full_delete_4_Btn, 22, 302);
		lv_obj_set_size(scr->Favorites_full_delete_4_Btn, 260, 52);

		// Set style of scr->Favorites_full_delete_4_Btn
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_4_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_1.png), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_font(scr->Favorites_full_delete_4_Btn, &c_taiwanpearl_regular_30, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_text_color(scr->Favorites_full_delete_4_Btn, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_border_width(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_obj_set_style_bg_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_bg_img_src(scr->Favorites_full_delete_4_Btn, LVGL_IMAGE_PATH(Favorites_Full_Delete_2.png), LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_border_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_opa(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_ofs_y(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
		lv_obj_set_style_shadow_width(scr->Favorites_full_delete_4_Btn, 0, LV_PART_MAIN | LV_STATE_FOCUSED);
	}
	
	scr->obj_null = lv_obj_create(scr->obj);
    lv_obj_remove_flag(scr->obj_null, LV_OBJ_FLAG_CLICKABLE);          // 1. 不可交互
    lv_obj_add_flag(scr->obj_null, LV_OBJ_FLAG_HIDDEN);                // 2. 隐藏
    lv_obj_set_size(scr->obj_null, 1, 1);                              // 3. 最小尺寸
    scr->obj_null_1 = lv_obj_create(scr->obj);
    lv_obj_remove_flag(scr->obj_null_1, LV_OBJ_FLAG_CLICKABLE);          // 1. 不可交互
    lv_obj_add_flag(scr->obj_null_1, LV_OBJ_FLAG_HIDDEN);                // 2. 隐藏
    lv_obj_set_size(scr->obj_null_1, 1, 1);                              // 3. 最小尺寸
    scr->obj_null_2 = lv_obj_create(scr->obj);
    lv_obj_remove_flag(scr->obj_null_2, LV_OBJ_FLAG_CLICKABLE);          // 1. 不可交互
    lv_obj_add_flag(scr->obj_null_2, LV_OBJ_FLAG_HIDDEN);                // 2. 隐藏
    lv_obj_set_size(scr->obj_null_2, 1, 1);                              // 3. 最小尺寸
    scr->obj_null_3 = lv_obj_create(scr->obj);
    lv_obj_remove_flag(scr->obj_null_3, LV_OBJ_FLAG_CLICKABLE);          // 1. 不可交互
    lv_obj_add_flag(scr->obj_null_3, LV_OBJ_FLAG_HIDDEN);                // 2. 隐藏
    lv_obj_set_size(scr->obj_null_3, 1, 1);                              // 3. 最小尺寸
	scr->obj_null_4 = lv_obj_create(scr->obj);
    lv_obj_remove_flag(scr->obj_null_4, LV_OBJ_FLAG_CLICKABLE);          // 1. 不可交互
    lv_obj_add_flag(scr->obj_null_4, LV_OBJ_FLAG_HIDDEN);                // 2. 隐藏
    lv_obj_set_size(scr->obj_null_4, 1, 1);                              // 3. 最小尺寸


	//焦点组添加对象
    if(scr->group == NULL && scr->group_sub_1 == NULL && scr->group_sub_2 == NULL && scr->group_sub_3 == NULL)
    {
        scr->group = lv_group_create();        // 创建焦点组对象
        scr->group_sub_1 = lv_group_create();        // 创建焦点组对象
        scr->group_sub_2 = lv_group_create();        // 创建焦点组对象
        scr->group_sub_3 = lv_group_create();        // 创建焦点组对象
		scr->group_sub_4 = lv_group_create();        // 创建焦点组对象
    }
    lv_group_add_obj(scr->group, scr->favorites_box_1_Btn);
    lv_group_add_obj(scr->group, scr->favorites_box_2_Btn);
	lv_group_add_obj(scr->group, scr->favorites_box_3_Btn);
    lv_group_add_obj(scr->group, scr->favorites_box_4_Btn);
	if (Del_Fav_create_flag)	//创建删除界面
	{
		lv_group_add_obj(scr->group_sub_1, scr->Favorites_full_delete_1_Btn);
		lv_group_add_obj(scr->group_sub_2, scr->Favorites_full_delete_2_Btn);
		lv_group_add_obj(scr->group_sub_3, scr->Favorites_full_delete_3_Btn);
		lv_group_add_obj(scr->group_sub_4, scr->Favorites_full_delete_4_Btn);
	}
	else
	{
		lv_group_add_obj(scr->group_sub_1, scr->favo_Start_Btn_1);
		lv_group_add_obj(scr->group_sub_1, scr->favo_Delete_Btn_1);
		lv_group_add_obj(scr->group_sub_2, scr->favo_Start_Btn_2);
		lv_group_add_obj(scr->group_sub_2, scr->favo_Delete_Btn_2);
		lv_group_add_obj(scr->group_sub_3, scr->favo_Start_Btn_3);
		lv_group_add_obj(scr->group_sub_3, scr->favo_Delete_Btn_3);
		lv_group_add_obj(scr->group_sub_4, scr->favo_Start_Btn_4);
		lv_group_add_obj(scr->group_sub_4, scr->favo_Delete_Btn_4);
	}
	lv_group_add_obj(scr->group, scr->obj_null);
    lv_group_add_obj(scr->group_sub_1, scr->obj_null_1);
    lv_group_add_obj(scr->group_sub_2, scr->obj_null_2);
    lv_group_add_obj(scr->group_sub_3, scr->obj_null_3);
	lv_group_add_obj(scr->group_sub_4, scr->obj_null_4);

    lv_group_set_default(scr->group);
	lv_group_focus_obj(scr->obj_null);


    lv_obj_add_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
	lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+

	favorites_how_many = 0;	//按当前收藏集合重新统计数量（探针/非探针各自独立）

    if (Fav_Cur->has_favorites_byte & 0b1)
    {
        favorites_how_many = 1;
		lv_group_focus_obj(scr->favorites_box_1_Btn);
		lv_obj_add_flag(scr->plus_sign_1, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(scr->favo_Cont_1, LV_OBJ_FLAG_HIDDEN);     	//显示收藏一内容
        lv_group_focus_obj(scr->obj_null_1);
    }
    if (Fav_Cur->has_favorites_byte & 0b10)
    {
		favorites_how_many = 2;

		lv_obj_add_flag(scr->plus_sign_2, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_2, LV_OBJ_FLAG_HIDDEN);     	//显示收藏二内容
        lv_group_focus_obj(scr->obj_null_2);
    }
    if (Fav_Cur->has_favorites_byte & 0b100)
    {
		favorites_how_many = 3;

		lv_obj_add_flag(scr->plus_sign_3, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
		lv_obj_remove_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);     		//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_3, LV_OBJ_FLAG_HIDDEN);     	//显示收藏三内容
        lv_group_focus_obj(scr->obj_null_3);
    }
	if (Fav_Cur->has_favorites_byte & 0b1000)
    {
		favorites_how_many = 4;

		lv_obj_add_flag(scr->plus_sign_4, LV_OBJ_FLAG_HIDDEN);      	//隐藏加号+
        lv_obj_remove_flag(scr->favo_Cont_4, LV_OBJ_FLAG_HIDDEN);     	//显示收藏四内容
        lv_group_focus_obj(scr->obj_null_4);
    }
	if (Fav_Cur->has_favorites_byte & 0b10000) 		favorites_how_many = 5;
	if (Fav_Cur->has_favorites_byte & 0b100000) 	favorites_how_many = 6;
	if (Fav_Cur->has_favorites_byte & 0b1000000) 	favorites_how_many = 7;
	if (Fav_Cur->has_favorites_byte & 0b10000000) 	favorites_how_many = 8;
	
	if (favorites_how_many < 5)
	{
		lv_obj_add_flag(scr->fav_page_1_No, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->fav_page_1_yes, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->fav_page_2_No, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(scr->fav_page_2_yes, LV_OBJ_FLAG_HIDDEN);
	}
}



