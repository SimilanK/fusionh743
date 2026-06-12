#include "airbrake_app.h"
#include "control.h"
#include "pseudo_traj.h"
#include "ff.h"
#include "main.h" // For UART and other HAL helpers if needed
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart2;

uint8_t app_done = 0;
static double t = 0.0;
static FIL SimLogFile;
static uint8_t sim_log_ready = 0;
static uint16_t row_count = 0;

void app_init(void) {
    // Initialize controller structure
    control_initialize();
    
    // Ingest the trajectory CSV from SD card
    HAL_UART_Transmit(&huart2, (uint8_t *)"Loading pseudo_flight.csv...\r\n", 30, HAL_MAX_DELAY);
    uint8_t loaded = pseudo_traj_load();
    if (!loaded) {
        HAL_UART_Transmit(&huart2, (uint8_t *)"ERROR: Failed to load pseudo_flight.csv!\r\n", 42, HAL_MAX_DELAY);
        app_done = 1;
        return;
    }
    HAL_UART_Transmit(&huart2, (uint8_t *)"pseudo_flight.csv loaded successfully into RAM.\r\n", 49, HAL_MAX_DELAY);
    
    // Open output log file on SD Card
    FRESULT fres = f_open(&SimLogFile, "SIM01.CSV", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres == FR_OK) {
        sim_log_ready = 1;
        const char *header = "t,h,vz,logic,Out1,z1,z2,z3,u_prev\r\n";
        UINT bw;
        f_write(&SimLogFile, header, strlen(header), &bw);
        f_sync(&SimLogFile);
    } else {
        HAL_UART_Transmit(&huart2, (uint8_t *)"ERROR: Failed to open SIM01.CSV for logging!\r\n", 46, HAL_MAX_DELAY);
    }
    
    t = 0.0;
    app_done = 0;
    row_count = 0;
}

void app_tick(void) {
    if (app_done) {
        return;
    }
    
    double h = 0.0;
    double vz = 0.0;
    double pitch = 0.0;
    boolean_T logic_val = false;
    
    // Sample the decimated trajectory at the current time t
    uint8_t finished = pseudo_sample(t, &h, &vz, &pitch, &logic_val);
    
    // Assign to inputs (cast up to double/real_T as required)
    control_U.h = (real_T)h;
    control_U.Vz = (real_T)vz;
    control_U.pitch = (real_T)pitch;
    control_U.logic = logic_val;
    
    // Run multirate scheduling call sequence at 1 kHz base rate
    boolean_T eventFlags[2] = {false, false};
    control_SetEventsForThisBaseStep(eventFlags);
    control_step0();
    if (eventFlags[1]) {
        control_step1();
    }
    
    // Write out states and controller signals to SIM01.CSV
    if (sim_log_ready) {
        char buf[160];
        int len = snprintf(buf, sizeof(buf), 
            "%.3f,%.3f,%.3f,%d,%.6f,%.6f,%.6f,%.6f,%.6f\r\n",
            t, 
            control_U.h, 
            control_U.Vz, 
            control_U.logic ? 1 : 0,
            control_Y.Out1,
            control_DW.z1,
            control_DW.z2,
            control_DW.z3,
            control_DW.u_prev
        );
        
        if (len > 0) {
            UINT bw;
            f_write(&SimLogFile, buf, len, &bw);
            row_count++;
            
            // Flush to SD card every 50 ticks to balance speed and data safety
            if (row_count >= 50) {
                f_sync(&SimLogFile);
                row_count = 0;
            }
        }
    }
    
    // Advance simulation time (1 ms step size)
    t += 0.001;
    
    if (finished) {
        app_finish();
    }
}

void app_finish(void) {
    if (sim_log_ready) {
        f_sync(&SimLogFile);
        f_close(&SimLogFile);
        sim_log_ready = 0;
    }
    app_done = 1;
}
