
/*代码思路写在了结尾，那个oled屏幕的调用函数我不会写（）*/
#include "key.h"
Key_HandleTypeDef key_list[7]={
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_0, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_1, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_2, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_3, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_4, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOA, .gpio_pin = GPIO_PIN_5, .press_flag = 0, .press_callback=NULL},
{.key_state = 0, .filter_cnt = 0, .gpio_port = GPIOB, .gpio_pin = GPIO_PIN_0, .press_flag = 0, .press_callback=NULL},
};
  void Key_Update(void)
{ 
  for(uint8_t i=0;i<7;i++)
    {
      uint8_t current_pin_level=HAL_GPIO_ReadPin(key_list[i].gpio_port,key_list[i].gpio_pin);
      /*判断某个引脚的高低电平*/
      if(current_pin_level==GPIO_PIN_RESET)
      {
        if(key_list[i].filter_cnt<10)//用10ms去判断是真的按下还是抖动
        {
          key_list[i].filter_cnt++;//若真的按下，cnt变量将加到10
        }
        else
        {
          if (key_list[i].key_state==0)//若cnt变量达到10，执行到这里，判断之前的状态是不是按下，以防hold长条，若不是则执行
          {
            key_list[i].key_state=1;
            key_list[i].press_flag=1;
            if(key_list[i].press_callback!=NULL)
            {
              key_list[i].press_callback();
            }
            
          }
        }
      }
      else
      {
         if(key_list[i].filter_cnt > 0)
          {
            key_list[i].filter_cnt--;
          }
          else
          {
            key_list[i].key_state = 0;
            key_list[i].press_flag = 0; // 松开后清除按下标志位
          }
      }
    }
}
/*大概思路是先判断引脚是否为低电平，
是：
  给filter_cnt加值，1ms加1
    判断按下时间是否超过10ms?
      不是：按键的抖动，通过复位操作回位
      是：判断到按键确实在被按下
        判断是否是长按？
          是：不做状态变化
          不是：变化状态


不是：
  1.松开未复位，此时要把filter_cnt变量复位
  2.根本没按下，那就维持原样
*/
