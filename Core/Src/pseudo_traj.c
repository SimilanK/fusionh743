#include "pseudo_traj.h"
#include "ff.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POINTS 1000
#define DECIMATION_FACTOR 50

typedef struct {
    float t;
    float h;
    float vz;
    float pitch;
    uint8_t logic;
} TrajPoint_t;

static TrajPoint_t traj_table[MAX_POINTS];
static uint32_t num_points = 0;

uint8_t pseudo_traj_load(void) {
    FIL file;
    FRESULT fr = f_open(&file, "pseudo_flight.csv", FA_READ);
    if (fr != FR_OK) {
        return 0; // Failed to open file
    }
    
    char line[128];
    uint32_t valid_rows = 0;
    num_points = 0;
    float t_offset = 0.0f;
    uint8_t first_row = 1;
    
    while (f_gets(line, sizeof(line), &file) != NULL) {
        float time_val, h_val, vz_val, logic_val, pitch_val;
        
        // Parse CSV row: time, h, vz, logic, pitch
        int n = sscanf(line, "%f,%f,%f,%f,%f", &time_val, &h_val, &vz_val, &logic_val, &pitch_val);
        if (n < 5) {
            continue; // Parse error or incomplete row
        }
        
        // Filter out initial all-zero or noise rows where altitude is too small
        if (h_val < 100.0f) {
            continue;
        }
        
        if (first_row) {
            t_offset = time_val;
            first_row = 0;
        }
        
        // Decimate rows
        if (valid_rows % DECIMATION_FACTOR == 0) {
            if (num_points < MAX_POINTS) {
                traj_table[num_points].t = time_val - t_offset;
                traj_table[num_points].h = h_val;
                traj_table[num_points].vz = vz_val;
                traj_table[num_points].pitch = pitch_val;
                traj_table[num_points].logic = (logic_val != 0.0f) ? 1 : 0;
                num_points++;
            } else {
                break; // Buffer full
            }
        }
        valid_rows++;
    }
    
    f_close(&file);
    return (num_points > 0) ? 1 : 0;
}

uint8_t pseudo_sample(double t, double *h, double *vz, double *pitch, boolean_T *logic) {
    if (num_points == 0) {
        *h = 892.0;
        *vz = 0.0;
        *pitch = -0.104719755;
        *logic = false;
        return 1; // Not loaded
    }
    
    // Bounds check: before start of table
    if (t <= (double)traj_table[0].t) {
        *h = traj_table[0].h;
        *vz = traj_table[0].vz;
        *pitch = traj_table[0].pitch;
        *logic = (boolean_T)traj_table[0].logic;
        return 0;
    }
    
    // Bounds check: after end of table
    if (t >= (double)traj_table[num_points - 1].t) {
        *h = traj_table[num_points - 1].h;
        *vz = traj_table[num_points - 1].vz;
        *pitch = traj_table[num_points - 1].pitch;
        *logic = (boolean_T)traj_table[num_points - 1].logic;
        return 1; // Finished
    }
    
    // Binary search for interval
    uint32_t low = 0;
    uint32_t high = num_points - 1;
    uint32_t idx = 0;
    
    while (low <= high) {
        uint32_t mid = (low + high) / 2;
        if (t >= (double)traj_table[mid].t) {
            idx = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    
    if (idx >= num_points - 1) {
        idx = num_points - 2;
    }
    
    double t0 = traj_table[idx].t;
    double t1 = traj_table[idx + 1].t;
    double ratio = (t - t0) / (t1 - t0);
    
    *h = (double)traj_table[idx].h + ratio * ((double)traj_table[idx + 1].h - (double)traj_table[idx].h);
    *vz = (double)traj_table[idx].vz + ratio * ((double)traj_table[idx + 1].vz - (double)traj_table[idx].vz);
    *pitch = (double)traj_table[idx].pitch + ratio * ((double)traj_table[idx + 1].pitch - (double)traj_table[idx].pitch);
    *logic = (ratio >= 0.5) ? (boolean_T)traj_table[idx + 1].logic : (boolean_T)traj_table[idx].logic;
    
    return 0;
}
