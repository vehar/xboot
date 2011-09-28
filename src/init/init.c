/*
 * init/init.c
 *
 * Copyright (c) 2007-2008  jianjun jiang <jerryjianjun@gmail.com>
 * official site: http://xboot.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <xboot.h>
#include <types.h>
#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <time/tick.h>
#include <time/timer.h>
#include <time/delay.h>
#include <time/xtime.h>
#include <xboot/log.h>
#include <xboot/list.h>
#include <xboot/menu.h>
#include <xboot/printk.h>
#include <console/console.h>
#include <fs/fileio.h>
#include <init.h>

/*
 * mount root filesystem and create some directory.
 */
void do_system_rootfs(void)
{
	LOG_I("mount root filesystem");

	if(mount(NULL, "/" , "ramfs", 0) != 0)
		LOG_E("failed to mount root filesystem");

	if(chdir("/") != 0)
		LOG_E("can't change directory to '/'");

	if(mkdir("/proc", S_IRUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0)
		LOG_E("failed to create directory '/proc'");

	if(mount(NULL, "/proc" , "procfs", 0) != 0)
		LOG_E("failed to mount proc filesystem");

	if(mkdir("/dev", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0)
		LOG_E("failed to create directory '/dev'");

	if(mount(NULL, "/dev" , "devfs", 0) != 0)
		LOG_E("failed to mount dev filesystem");

	if(mkdir("/romdisk", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0)
		LOG_E("failed to create directory '/romdisk'");

	if(mount("/dev/romdisk", "/romdisk" , "cpiofs", 0) != 0)
		LOG_E("failed to mount romdisk");

	if(mkdir("/etc", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0)
		LOG_E("failed to create directory '/etc'");

	if(mkdir("/tmp", S_IRWXU|S_IRWXG|S_IRWXO) != 0)
		LOG_E("failed to create directory '/tmp'");

	if(mkdir("/mnt", S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH) != 0)
		LOG_E("failed to create directory '/mnt'");
}

/*
 * do load system configure
 */
void do_system_cfg(void)
{
	LOG_I("load system configure");

	/*
	 * load the setting of stdin, stdout and stderr console
	 */
	if(! console_stdio_load("/etc/console.xml"))
	{
		if(! console_stdio_load("/romdisk/etc/console.xml"))
			LOG_E("can not setting the standard console");
	}

	/*
	 * load environment variable
	 */
	if(! loadenv("/etc/environment.xml"))
	{
		if(! loadenv("/romdisk/etc/environment.xml"))
			LOG_E("can not load environment variable");
	}

	/*
	 * load menu context
	 */
	if(! menu_load("/etc/menu.xml"))
	{
		if(! menu_load("/romdisk/etc/menu.xml"))
			LOG_E("can not load menu context");
	}
}

/*
 * do load system fonts
 */
void do_system_fonts(void)
{
	char path[MAX_PATH];
	char buf[MAX_PATH];
	struct stat st;
	struct dirent * entry;
	void * dir;

	LOG_I("load system fonts");

	/*
	 * system fonts's directory path
	 */
	sprintf(path, "%s", "/romdisk/fonts");

	if(stat(path, &st) != 0)
		return;

	if(! S_ISDIR(st.st_mode))
		return;

	if( (dir = opendir(path)) == NULL)
    	return;

    for(;;)
    {
		if( (entry = readdir(dir)) == NULL)
		  break;

		buf[0] = 0;
		strlcpy(buf, path, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';

		if(!strcmp(entry->d_name, "."))
		{
			continue;
		}
		else if(!strcmp(entry->d_name, ".."))
		{
			continue;
		}
		else
		{
			strlcat(buf, "/", sizeof(buf));
			strlcat(buf, entry->d_name, sizeof(buf));
		}

		if(stat(buf, &st) != 0)
			continue;

		/*
		 * FIXME
		 */
		// if(! install_font(buf))
		//	LOG_E("fail to install font: %s", buf);
    }

	closedir(dir);
}

/*
 * wait a moment (two seconds)
 */
void do_system_wait(void)
{
	u32_t div, rem;

	if(get_system_hz() > 0)
	{
		div = jiffies / get_system_hz();
		rem = jiffies % get_system_hz();

		LOG_I("wait a moment, if necessary (%lu.%02lu)", div, rem);

		/*
		 * wait a moment for uptime until two seconds
		 */
		while(jiffies < get_system_hz() * 2);
	}
}
