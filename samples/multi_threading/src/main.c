
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define PIN_THREADS (IS_ENABLED(CONFIG_SMP) && IS_ENABLED(CONFIG_SCHED_CPU_MASK))

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

/* LED BLINKING TIME (in ms) */
#define BLINKTIME 500  // = 1 Hz 

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);

void led_blink()
{
	uint8_t cpu;

	int ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
    	printk("GPIO configure failed: %d\n", ret);
    	return;
	}

	while (1) {
#if CONFIG_SMP
		cpu = arch_curr_cpu()->id;
#else
		cpu = 0;
#endif
		gpio_pin_toggle_dt(&led0);
		int state = gpio_pin_get_dt(&led0);
		printk("LED state: %s on cpu %d \n", state ? "ON" : "OFF", cpu);
		k_msleep(BLINKTIME);
	}
}

void uptime()
{
	uint8_t cpu;

	while (1) {
#if CONFIG_SMP
		cpu = arch_curr_cpu()->id;
#else
		cpu = 0;
#endif

		printk("Time elapsed on cpu %d: %lld\n",cpu,k_uptime_get());
		k_msleep(1000); // Delaying it for 1 sec.
	}
}


void led_blink_entry_point(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	led_blink();
}


void time_thread_entry_point(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	uptime();
	
}
K_THREAD_DEFINE(led_blink_thread, STACKSIZE,
				led_blink_entry_point, NULL, NULL, NULL,
				PRIORITY, 0, 0);

K_THREAD_DEFINE(time_thread, STACKSIZE,
				time_thread_entry_point, NULL, NULL, NULL,
				PRIORITY, 0, 0);


int main(void)
{
	k_msleep(2000); // Delaying it for 2 sec to see on which core the scheduler scheduled the thread
#if PIN_THREADS
	if (arch_num_cpus() > 1) {

		k_thread_suspend(led_blink_thread);
		k_thread_cpu_pin(led_blink_thread, 2);
		k_thread_resume(led_blink_thread);

		k_thread_suspend(time_thread);
		k_thread_cpu_pin(time_thread, 3);
		k_thread_resume(time_thread);
	}
#endif
	return 0;
}
