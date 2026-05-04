/* ----------------------------------- FIXED COMMON CONSTANTS ----------------------------------- */
/* ----------------------------------- ===== ====== ========= ----------------------------------- */
/* ----------------------------------- ===== ====== ========= ----------------------------------- */

PRC_FAULT_TABLE_WIDTH 6
PRC_FAULT_TABLE_ADDR (~(((word)(1<<(PRC_FAULT_TABLE_WIDTH + 2))) - 1))

umax cache_fetch_costs(index level); // Should return a valid value for PRC_CACHES + 1(for primary memory) levels

index cache_sizes(index level); // Should return a valid value for PRC_CACHES levels

umax rob_periodicity(index priority); // How many cycles should elapse between fetching into a particular ROB, based on it's priority
