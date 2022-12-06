#pragma once

#define RMIDX(row, col, nrow, ncol) ((row) * (ncol) + (col)) /* row major index */
#define CMIDX(row, col, nrow, ncol) ((row) + (nrow) * (col)) /* col major index */
#define IDX3D(x, y, z, ldx, ldy) ((x) + (ldx) * (y) + (ldx) * (ldy) * (z))
