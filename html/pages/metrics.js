function CodeMetrics() {
	 this.metricsArray = {};
	 this.metricsArray.var = new Array();
	 this.metricsArray.fcn = new Array();
	 this.metricsArray.var["__env"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	size: 4};
	 this.metricsArray.var["environ"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	size: 4};
	 this.metricsArray.fcn["__errno_location"] = {file: "C:\\Program Files\\MATLAB\\R2024a\\polyspace\\verifier\\cxx\\include\\include-libc\\bits\\errno.h",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["__io_getchar"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["__io_putchar"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 0,
	stackTotal: 0};
	 this.metricsArray.fcn["_close"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 8,
	stackTotal: 8};
	 this.metricsArray.fcn["_execve"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 16,
	stackTotal: 16};
	 this.metricsArray.fcn["_exit"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["_fork"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["_fstat"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 12,
	stackTotal: 12};
	 this.metricsArray.fcn["_getpid"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 4,
	stackTotal: 4};
	 this.metricsArray.fcn["_isatty"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 8,
	stackTotal: 8};
	 this.metricsArray.fcn["_kill"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 12,
	stackTotal: 12};
	 this.metricsArray.fcn["_link"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 12,
	stackTotal: 12};
	 this.metricsArray.fcn["_lseek"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 16,
	stackTotal: 16};
	 this.metricsArray.fcn["_open"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 12,
	stackTotal: 12};
	 this.metricsArray.fcn["_read"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 20,
	stackTotal: 20};
	 this.metricsArray.fcn["_stat"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 12,
	stackTotal: 12};
	 this.metricsArray.fcn["_times"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 8,
	stackTotal: 8};
	 this.metricsArray.fcn["_unlink"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 8,
	stackTotal: 8};
	 this.metricsArray.fcn["_wait"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 8,
	stackTotal: 8};
	 this.metricsArray.fcn["_write"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 20,
	stackTotal: 20};
	 this.metricsArray.fcn["initialise_monitor_handles"] = {file: "C:\\Users\\simil\\STM32CubeIDE\\workspace_2.1.1\\PILH743\\STM32CubeIDE\\Application\\User\\Core\\syscalls.c",
	stack: 0,
	stackTotal: 0};
	 this.getMetrics = function(token) { 
		 var data;
		 data = this.metricsArray.var[token];
		 if (!data) {
			 data = this.metricsArray.fcn[token];
			 if (data) data.type = "fcn";
		 } else { 
			 data.type = "var";
		 }
	 return data; }; 
	 this.codeMetricsSummary = '<a href="javascript:void(0)" onclick="return postParentWindowMessage({message:\'gotoReportPage\', pageName:\'control_metrics\'});">Global Memory: 8(bytes) Maximum Stack: 20(bytes)</a>';
	}
CodeMetrics.instance = new CodeMetrics();
