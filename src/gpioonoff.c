#define _GNU_SOURCE
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#define GPIO_PATH "/sys/class/gpio/"

#ifndef BUZZER_PWM_INTERFACE
#define BUZZER_PWM_INTERFACE "/sys/class/hwmon/hwmon1/device/pwm1"
#endif
#ifndef BEEP_DUTY 
#define BEEP_DUTY "300"
#endif

#ifndef BACKLIGHTPOWERSYSFILE
#define BACKLIGHTPOWERSYSFILE "/sys/class/backlight/backlight/bl_power"
#endif

#warning still not all warnings fixed


int
set_gpio_toval (int gpionr, int val)
{
	char *filename, *tmp;
	FILE *fh;

	printf("Ill try to set [%d] ", gpionr);
	asprintf (&filename, GPIO_PATH "gpio%d/value", gpionr);

	printf(":  %s\n",filename);
	fh = fopen (filename, "w");
	if (fh == NULL)
	{
		fh = fopen (GPIO_PATH "/export", "w");
		if ( fh == NULL) 
		{
			printf("Can't open export\n");
			return -1;
		}
		fprintf(fh, "%d\n", gpionr);
		fclose (fh);
		asprintf (&tmp, GPIO_PATH "gpio%d/direction", gpionr);
		fh = fopen (tmp, "w");
		free (tmp);
		if (fh == NULL) {
			printf("Can't change direction of pin %d\n", gpionr);
			return -1;
		}
		fprintf(fh, "out");
		fclose (fh);
		fh = fopen (filename, "w");
		if (fh == NULL ) {
			printf("Can't open gpio-pin %d\n", gpionr);
			return -1;
		}
	}
	fprintf(fh, "%d\n", val);
	fclose (fh);
	free (filename);
	return 0;
}

int fump_all(int val){
   int i;
   for (i=227;i<245;i++){
       set_gpio_toval(i, val);
	usleep(100);
   }
   return 0;
}

/*
After kernel-update there is the new userspace-pwm API available.
In /sys/class/hwmon/hwmon1/device is a file pwm1 and pwm1_freq. pwm1_freq gets
the frequency of the timer, pwm1 the duty-cylce (from 0-1000).
*/

int beep_narf(int freq , int ms_beep)
{
	FILE *fh;
	
	/*set frequency first*/
	fh = fopen(BUZZER_PWM_INTERFACE "_freq" , "w");
	if (fh == NULL ) {
		printf("Can't open freq file of buzzer pwm!\n");
		return -1;
	}
	fprintf(fh,"%d\n" , freq);
	fclose(fh);
	fh = fopen(BUZZER_PWM_INTERFACE,"w");
	if (fh == NULL ) {
		printf("Cant open pwm file of buzzer!\n");
		return -1;
	}
	fprintf(fh, BEEP_DUTY "\n");
	fclose(fh);
	
	usleep(ms_beep * 1000);

	fh = fopen(BUZZER_PWM_INTERFACE,"w");
	if (fh == NULL ) {
		printf("Cant open pwm file of buzzer!\n");
		return -1;
	}
	fprintf(fh, "0\n");
	fclose(fh);
}

/**
 * write state to /sys/class/backlight/backlight/bl_power
 * Attention: 0 means backlight on!
 */
int set_backlight_power(int state){
	FILE *fh;
	fh  = fopen(BACKLIGHTPOWERSYSFILE , "w");
	if (fh == NULL) {
		printf("Cant open backlight power control file!\n");
		/*  it might be bad to report error on a screen, 
		 * that couldn't be read due to the error.
		 * syslog might be the right place. */
		return -1;
	}
	fputs((state)?"1":"0",fh);
	fclose(fh);
}



#ifdef FREESTAND_NARF
int
main (int argc, char *argv[])
{
	int i, a;
	if (argc < 2)
	{
		printf ("usage: %s <gpionr> [<gpionr>]* <value> \n"
			"     | %s <value> \n "
			"for the second invocation pattern a value of \n"
			" 0 means set particular pins 0 \n"
			" 1 means set particular pins 1 \n"
			" 2 means beep with buzzer	\n"
			, argv[0],argv[0]);
		exit (1);
	}
	if (argc == 2)
	{
		if (atoi(argv[1]) == 2)
			return	beep_narf(1000 , 100);
		if (atoi(argv[1]) == 4){
			printf("beebs!\n");

			beep_narf(1000,100);
			beep_narf(500,100);
			beep_narf(440,200);
			beep_narf(340,20);
			beep_narf(720,40);
			beep_narf(880,400);
			return 0;
		}
		return fump_all(atoi(argv[1]));
	}
	argc--;
	for (i = 1; i < (argc); i++)
	{
		a = set_gpio_toval (atoi (argv[i]),atoi(argv[argc]));
	}
	return 0;
}
#endif //FREESTAND_NARF

