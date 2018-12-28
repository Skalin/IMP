
#include "mouse.h"
#include <usb_hid.h>

extern void Main_Task(uint32_t param);
extern void mouseRun(uint32_t param);
#define MAIN_TASK       10
#define SETTINGS_COUNTER 5

TASK_TEMPLATE_STRUCT  MQX_template_list[] =
{
   { MAIN_TASK, mouseRun, 2000L, 7L, "Main", MQX_AUTO_START_TASK, 0, 0},
   { 0L, 0L, 0L, 0L, 0L, 0L , 0, 0}
};


typedef enum {
    BUTTON_NOT_INITIALIZED = -1,
    BUTTON_RELEASED,
    BUTTON_PRESSED
} button_state_t;

void int_service_routine(void *pin)
{
    lwgpio_int_clear_flag((LWGPIO_STRUCT_PTR) pin);
}



/****************************************************************************
 * Global Variables
 ****************************************************************************/
/* Add all the variables needed for mouse.c to this structure */
extern USB_ENDPOINTS usb_desc_ep;
extern DESC_CALLBACK_FUNCTIONS_STRUCT  desc_callback;
MOUSE_GLOBAL_VARIABLE_STRUCT g_mouse;

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/


void USB_App_Callback(uint8_t event_type, void* val,void *arg);
uint8_t USB_App_Param_Callback(uint8_t request, uint16_t value, uint8_t **data,
    uint32_t* size,void   *arg);



int getMouseDirection(int direction)
{
	if (direction == 1 || direction == 2)
	{
		return 1;
	}
	if (direction == 3 || direction == 4)
	{
		return -1;
	}
	return 0;
}

void sendMouseCommand(Mouse_settings ms)
{
    enum {NONE = -1, RIGHT = 1, DOWN = 2, LEFT = 3, UP = 4 };


    int sendUpdate = 0;

    g_mouse.rpt_buf[0] = 0;
    g_mouse.rpt_buf[1] = 0;
    g_mouse.rpt_buf[2] = 0;

    if (ms.MOUSE_DIR == RIGHT)
    {
		if (ms.SECOND_PRESS)
		{
			g_mouse.rpt_buf[2] = getMouseDirection(ms.MOUSE_SECOND_DIR)*ms.MOUSE_SPEED;
		}
		g_mouse.rpt_buf[1] = getMouseDirection(ms.MOUSE_DIR)*ms.MOUSE_SPEED;
		sendUpdate = 1;
	}
    if (ms.MOUSE_DIR == DOWN)
	{
    	if (ms.SECOND_PRESS)
		{
			g_mouse.rpt_buf[1] = getMouseDirection(ms.MOUSE_SECOND_DIR)*ms.MOUSE_SPEED;
		}
		g_mouse.rpt_buf[2] = getMouseDirection(ms.MOUSE_DIR)*ms.MOUSE_SPEED;
		sendUpdate = 1;
    }
    if (ms.MOUSE_DIR == LEFT)
    {
        if (ms.SECOND_PRESS)
		{
			g_mouse.rpt_buf[2] = getMouseDirection(ms.MOUSE_SECOND_DIR)*ms.MOUSE_SPEED;
		}
        g_mouse.rpt_buf[1] = (uint8_t)getMouseDirection(ms.MOUSE_DIR)*ms.MOUSE_SPEED;
		sendUpdate = 1;
    }
    if (ms.MOUSE_DIR == UP)
    {
    	if (ms.SECOND_PRESS)
		{
			g_mouse.rpt_buf[1] = getMouseDirection(ms.MOUSE_SECOND_DIR)*ms.MOUSE_SPEED;
		}
        g_mouse.rpt_buf[2] = (uint8_t)getMouseDirection(ms.MOUSE_DIR)*ms.MOUSE_SPEED;
		sendUpdate = 1;
    }

    if (ms.BUTTON_PRESS)
    {
    	g_mouse.rpt_buf[0] = ms.BUTTON;
		sendUpdate = 1;

    }

    if (sendUpdate)
    {
        (void)USB_Class_HID_Send_Data(g_mouse.app_handle,HID_ENDPOINT,
            g_mouse.rpt_buf,MOUSE_BUFF_SIZE);

    }

}

/******************************************************************************
 *
 *    @name        USB_App_Callback
 *
 *    @brief       This function handles the callback
 *
 *    @param       handle : handle to Identify the controller
 *    @param       event_type : value of the event
 *    @param       val : gives the configuration value
 *
 *    @return      None
 *
 *****************************************************************************/
void USB_App_Callback(uint8_t event_type, void* val,void *arg)
{
    UNUSED_ARGUMENT (arg)
    UNUSED_ARGUMENT (val)

    switch(event_type)
    {
        case USB_APP_BUS_RESET:
            g_mouse.mouse_init=FALSE;
            break;
        case USB_APP_ENUM_COMPLETE:
            g_mouse.mouse_init = TRUE;
            //move_mouse();/* run the coursor movement code */
            break;
        case USB_APP_SEND_COMPLETE:
             /*check whether enumeration is complete or not */
            if(g_mouse.mouse_init)
            {
                #if COMPLIANCE_TESTING
                    uint32_t g_compliance_delay = 0x009FFFFF;
                    while(g_compliance_delay--);
                #endif
                //move_mouse();/* run the cursor movement code */
            }
            break;
        case USB_APP_ERROR:
            /* user may add code here for error handling
               NOTE : val has the value of error from h/w*/
            break;
        default:
            break;
    }


    return;
}

/******************************************************************************
 *
 *    @name        USB_App_Param_Callback
 *
 *    @brief       This function handles the callback for Get/Set report req
 *
 *    @param       request  :  request type
 *    @param       value    :  give report type and id
 *    @param       data     :  pointer to the data
 *    @param       size     :  size of the transfer
 *
 *    @return      status
 *                  USB_OK  :  if successful
 *                  else return error
 *
 *****************************************************************************/
 uint8_t USB_App_Param_Callback
 (
    uint8_t request,
    uint16_t value,
    uint8_t **data,
    uint32_t* size,
    void   *arg
)
{
    uint8_t error = USB_OK;
    //uint8_t direction =  (uint8_t)((request & USB_HID_REQUEST_DIR_MASK) >>3);
    uint8_t index = (uint8_t)((request - 2) & USB_HID_REQUEST_TYPE_MASK);

    UNUSED_ARGUMENT(arg)
    /* index == 0 for get/set idle, index == 1 for get/set protocol */
    *size = 0;
    /* handle the class request */
    switch(request)
    {
        case USB_HID_GET_REPORT_REQUEST :
            *data = &g_mouse.rpt_buf[0]; /* point to the report to send */
            *size = MOUSE_BUFF_SIZE; /* report size */
            break;

        case USB_HID_SET_REPORT_REQUEST :
            for(index = 0; index < MOUSE_BUFF_SIZE ; index++)
            {   /* copy the report sent by the host */
                g_mouse.rpt_buf[index] = *(*data + index);
            }
            break;

        case USB_HID_GET_IDLE_REQUEST :
            /* point to the current idle rate */
            *data = &g_mouse.app_request_params[index];
            *size = REQ_DATA_SIZE;
            break;

        case USB_HID_SET_IDLE_REQUEST :
            /* set the idle rate sent by the host */
            g_mouse.app_request_params[index] =(uint8_t)((value & MSB_MASK) >>
                HIGH_BYTE_SHIFT);
            break;

        case USB_HID_GET_PROTOCOL_REQUEST :
            /* point to the current protocol code
               0 = Boot Protocol
               1 = Report Protocol*/
            *data = &g_mouse.app_request_params[index];
            *size = REQ_DATA_SIZE;
            break;

        case USB_HID_SET_PROTOCOL_REQUEST :
            /* set the protocol sent by the host
               0 = Boot Protocol
               1 = Report Protocol*/
               g_mouse.app_request_params[index] = (uint8_t)(value);
               break;
    }
    return error;
}


 void initMouseSpeed(Mouse_settings *ms, int speed)
 {
	    ms->MOUSE_SPEED = speed;
 }

 void initMouseButton(Mouse_settings *ms, int button)
 {
	 ms->BUTTON = button;
 }

 void initMouseSettings(Mouse_settings *ms)
 {
	    ms->MOUSE_DIR = -1;
	    ms->MOUSE_SECOND_DIR = -1;
	    ms->SECOND_PRESS = 0;
	    ms->BUTTON_PRESS = 0;
 }

void setDir(Mouse_settings *ms, int DIR)
{
	if (ms->SECOND_PRESS)
	{
		ms->MOUSE_SECOND_DIR = DIR;
	}
	else
	{
		ms->MOUSE_DIR = DIR;
		ms->SECOND_PRESS = 1;
	}
}

void pressMouseButton(Mouse_settings *ms)
{
	ms->BUTTON_PRESS = 1;
}

void print(char *string)
{
	printf("%s", string);
	printf("\r\n");
}

void printArrow(int menuItem, int line)
{
	if (menuItem == line)
	{
		printf(" <--");
	}
	print("");
}

void printDelimiter()
{
    print("=========================================");
}

void printWelcome(Mouse_settings *ms)
{
	printf("");
    printDelimiter();
    print("IMP - USB Demonstration");
    print("Author: Dominik Skala");
    print("Subject: IMP 2018/2019");
    printDelimiter();
    print("");
    printf("To enable settings mode, press BTN5 (SW6 for FITKIT) %d times in a row.\r\n", SETTINGS_COUNTER-1);
    printDelimiter();
    print("");

    print("Initial settings: ");

    printSettings(ms);
    print("");
    printDelimiter();
}

void printClear()
{

	printf("\r\n\r\n\r\n\r\n\r\n");
	printf("\r\n\r\n\r\n\r\n\r\n");
	printf("\r\n\r\n\r\n\r\n\r\n");
	printf("\r\n\r\n\r\n\r\n\r\n");
}

void addPosition(int *settings_position)
{
	*settings_position += 1;
	if (*settings_position > 3)
	{
		*settings_position = 1;
	}
}

void subtractPosition(int *settings_position)
{
	*settings_position -= 1;
	if (*settings_position < 1)
	{
		*settings_position = 3;
	}
}

char *getMouseMap(Mouse_settings *ms)
{
	if (ms->BUTTON == 1)
	{
		return "LEFT BUTTON";
	}
	return "NON-EXISTENT BUTTON";
}

void printSettings(Mouse_settings *ms)
{
    printDelimiter();
	print("Settings");
    printDelimiter();
    print("");
	printf("MOUSE SPEED: %d\r\n", ms->MOUSE_SPEED);
	printf("MOUSE BUTTON MAPPED AS: %s\r\n", getMouseMap(ms));
	printf("Press button BTN5 (SW6 on FITKIT) to return to Settings menu\r");

}

void settingsMode(Mouse_settings *ms, LWGPIO_STRUCT *upButton, LWGPIO_STRUCT *downButton, LWGPIO_STRUCT *controlButton)
{
	int btnup_position, btnup_last_position;
	int btndown_position, btndown_last_position;
	int btncontrol_position, btncontrol_last_position;

	int settings = 1;
	int render = 1;

	enum {NONE = 0, CURRENT_SETTINGS, DPI_SETTINGS, EXIT_SETTINGS};

	int currentPosition = NONE;

	printClear();
	while(settings)
	{
		if(render)
		{

			printClear();
		    printDelimiter();
			print("Settings");
		    printDelimiter();
			printf("Current Settings");printArrow(CURRENT_SETTINGS, currentPosition);
			printf("DPI Settings");printArrow(DPI_SETTINGS, currentPosition);
			printf("Exit settings mode");printArrow(EXIT_SETTINGS, currentPosition);
		}
		render = 0;

		updateButtonState(upButton, &btnup_position);
		updateButtonState(downButton, &btndown_position);
		updateButtonState(controlButton, &btncontrol_position);

		int upPress = buttonPressedAndReleased(upButton, &btnup_position, &btnup_last_position);
		int downPress = buttonPressedAndReleased(downButton, &btndown_position, &btndown_last_position);
		int controlPress = buttonPressedAndReleased(controlButton, &btncontrol_position, &btncontrol_last_position);
		if (upPress || downPress || controlPress)
		{

			if (upPress)
			{
				subtractPosition(&currentPosition);
			}
			if (downPress)
			{
				addPosition(&currentPosition);
			}
			if (controlPress && currentPosition != NONE)
			{
				if (currentPosition == EXIT_SETTINGS)
				{
					settings = 0;
					printClear();
					printClear();
					print("EXITING SETTINGS MODE");
			        _time_delay(1000);
					printClear();
				}
				else if (currentPosition == DPI_SETTINGS)
				{

					printf("Current DPI settings: %d \r", ms->MOUSE_SPEED);
					updateButtonState(controlButton, &btncontrol_position);
					while (!buttonPressedAndReleased(controlButton, &btncontrol_position, &btncontrol_last_position))
					{
						updateButtonState(downButton, &btndown_position);
						if (buttonPressedAndReleased(downButton, &btndown_position, &btndown_last_position))
						{
							ms->MOUSE_SPEED--;
							printf("Current DPI settings: %d \r", ms->MOUSE_SPEED);
						}
						updateButtonState(upButton, &btnup_position);
						if (buttonPressedAndReleased(upButton, &btnup_position, &btnup_last_position))
						{
							ms->MOUSE_SPEED++;
							printf("Current DPI settings: %d \r", ms->MOUSE_SPEED);
						}
						updateButtonState(controlButton, &btncontrol_position);
					}
				}
				else if (currentPosition == CURRENT_SETTINGS)
				{
					printClear();
					printSettings(ms);
					updateButtonState(controlButton, &btncontrol_position);
					while (!buttonPressedAndReleased(controlButton, &btncontrol_position, &btncontrol_last_position))
					{
						updateButtonState(controlButton, &btncontrol_position);
					}
				}
			}
			render = 1;
		}

	}

	printClear();
}

int buttonPressedAndReleased(LWGPIO_STRUCT *btn, int *btn_state, int *btn_last_state)
{

	if (*btn_state != *btn_last_state)
	{
		*btn_last_state = *btn_state;
		if (*btn_state == BUTTON_RELEASED)
		{
			return true;
		}
	}
	return false;
}

void updateButtonState(LWGPIO_STRUCT *btn, int *btn_state)
{

	if (buttonPressed(btn))
	{
		*btn_state = BUTTON_PRESSED;
	}
	else
	{
		*btn_state = BUTTON_RELEASED;
	}
}



int activeHigh()
{
	#if defined(BSP_BUTTONS_ACTIVE_HIGH)
		return true;
	#else
		return false;
	#endif
}

int buttonPressed(LWGPIO_STRUCT *btn)
{
	if (activeHigh())
	{
		return lwgpio_get_value(btn) == LWGPIO_VALUE_HIGH;
	}
	else
	{
		return lwgpio_get_value(btn) == LWGPIO_VALUE_LOW;
	}

}



int initButton(LWGPIO_STRUCT *btn, int button)
{


	/******************************************************************************
		Open the pin (BSP_BTN1) for input, initialize interrupt
		and set interrupt handler.
	******************************************************************************/
	/* opening pins for input */
	if (!lwgpio_init(btn, button, LWGPIO_DIR_INPUT, LWGPIO_VALUE_NOCHANGE))
	{
		_task_block();
		return false;
	}

	/* Some platforms require to setup MUX to IRQ functionality,
	for the rest just set MUXto GPIO functionality */
	uint8_t mux = 0;
	if (button == BSP_BUTTON1)
		mux = BSP_BUTTON1_MUX_GPIO;

	if (button == BSP_BUTTON2)
		mux = BSP_BUTTON2_MUX_GPIO;

	if (button == BSP_BUTTON3)
		mux = BSP_BUTTON3_MUX_GPIO;

	if (button == BSP_BUTTON4)
		mux = BSP_BUTTON4_MUX_GPIO;

	if (button == BSP_BUTTON5)
		mux = BSP_BUTTON5_MUX_GPIO;

	lwgpio_set_functionality(btn, mux);
	lwgpio_set_attribute(btn, activeHigh() ? LWGPIO_ATTR_PULL_DOWN : LWGPIO_ATTR_PULL_UP, LWGPIO_AVAL_ENABLE);

	/* enable gpio functionality for given pin, react on falling edge */
	if (!lwgpio_int_init(btn, LWGPIO_INT_MODE_FALLING))
	{
		_task_block();
		return false;
	}

	/* install gpio interrupt service routine */
	_int_install_isr(lwgpio_int_get_vector(btn), int_service_routine, (void *) btn);
	/* set the interrupt level, and unmask the interrupt in interrupt controller */
	_bsp_int_init(lwgpio_int_get_vector(btn), 3, 0, TRUE);

	return true;
}

void mouseRun(uint32_t param)
{

	UNUSED_ARGUMENT(param);
	HID_CONFIG_STRUCT   config_struct;

    /* initialize the Global Variable Structure */
    USB_mem_zero(&g_mouse, sizeof(MOUSE_GLOBAL_VARIABLE_STRUCT));
    USB_mem_zero(&config_struct, sizeof(HID_CONFIG_STRUCT));

    /* Initialize the USB interface */

    config_struct.ep_desc_data = &usb_desc_ep;
    config_struct.hid_class_callback.callback = USB_App_Callback;
    config_struct.hid_class_callback.arg = &g_mouse.app_handle;
    config_struct.param_callback.callback = USB_App_Param_Callback;
    config_struct.param_callback.arg = &g_mouse.app_handle;
    config_struct.desc_callback_ptr = &desc_callback;
    config_struct.desc_endpoint_cnt = HID_DESC_ENDPOINT_COUNT;
    config_struct.ep = g_mouse.ep;

    if (MQX_OK != _usb_device_driver_install(USBCFG_DEFAULT_DEVICE_CONTROLLER)) {
        printf("Driver could not be installed\n");
        return;
    }

    g_mouse.app_handle = USB_Class_HID_Init(&config_struct);

	LWGPIO_STRUCT btn1;
	LWGPIO_STRUCT btn2;
	LWGPIO_STRUCT btn3;
	LWGPIO_STRUCT btn4;
	LWGPIO_STRUCT btn5;

    if (!initButton(&btn1, BSP_BUTTON1))
    {
    	print("Button 1 init failed!");
    }

    if (!initButton(&btn2, BSP_BUTTON2))
	{
		print("Button 2 init failed!");
	}

    if (!initButton(&btn3, BSP_BUTTON3))
	{
		print("Button 3 init failed!");
	}

    if (!initButton(&btn4, BSP_BUTTON4))
	{
		print("Button 4 init failed!");
	}

    if (!initButton(&btn5, BSP_BUTTON5))
	{
		print("Button 5 init failed!");
	}


    int btn5_state, btn5_last_state;
    int btn5_press_count = 0;

    Mouse_settings mouse_settings;

    initMouseSettings(&mouse_settings);
    initMouseSpeed(&mouse_settings, 5);
    initMouseButton(&mouse_settings, 1);

    printWelcome(&mouse_settings);


    while (TRUE)
    {
        /* call the periodic task function */
        USB_HID_Periodic_Task();


        if (buttonPressed(&btn5))
		{
			btn5_state = BUTTON_PRESSED;
		}
		else
		{
			btn5_state = BUTTON_RELEASED;
		}


		if (btn5_state != btn5_last_state)
		{
			btn5_last_state = btn5_state;
			if (btn5_state == BUTTON_RELEASED)
			{
				btn5_press_count++;
				if (btn5_press_count == SETTINGS_COUNTER-1)
				{
					print("Press BTN5 (SW6 on FITKIT) again to enable settings mode.");
				}
				if (btn5_press_count >= SETTINGS_COUNTER)
				{
					settingsMode(&mouse_settings, &btn4, &btn2, &btn5);
					btn5_press_count = 0;
				}
				else
				{
					pressMouseButton(&mouse_settings);
				}
			}
		}


		if (buttonPressed(&btn1))
		{
			setDir(&mouse_settings, 1);
			btn5_press_count = 0;
		}
		if (buttonPressed(&btn2))
		{
			setDir(&mouse_settings, 2);
			btn5_press_count = 0;
		}
		if (buttonPressed(&btn3))
		{
			setDir(&mouse_settings, 3);
			btn5_press_count = 0;
		}
		if (buttonPressed(&btn4))
		{
			setDir(&mouse_settings, 4);
			btn5_press_count = 0;
		}

		sendMouseCommand(mouse_settings);
	    initMouseSettings(&mouse_settings);

	    // check buttons every 5 ms
        _time_delay(5);
    }

}
/* EOF */
