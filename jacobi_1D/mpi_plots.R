library(ggplot2)
library(plyr)
library(dplyr)
library(data.table)
library(extrafont)

extrafont::loadfonts()

setwd("E:/Work/ETHZ/hpc/project/")

source("R/utils.R")
source("R/stats.R")

ghost_runs_file <- "results/1222155458_lsb_runs.csv"
runs_file <- "results/1223010000_lsb_runs.csv"
rank_file <- "results/1223010000_lsb_rank.csv"
runs2d_file <- "results/1223050254_lsb_runs.csv"

#Load data
ghost_runs_data <- read.table(ghost_runs_file, header = TRUE, sep = ',')
ghost_runs_data <- CalculateDataSummary(data=ghost_runs_data, measurevar="effective_time",
                                        groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                        conf.interval=.95, quantile.interval=.95)

runs_data_raw <- read.table(runs_file, header = TRUE, sep = ',')
runs_data <- CalculateDataSummary(data=runs_data_raw, measurevar="effective_time",
                                  groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                  conf.interval=.95, quantile.interval=.95)
runs_data_kernel <- CalculateDataSummary(data=runs_data_raw, measurevar="kernel_time",
                                  groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                  conf.interval=.95, quantile.interval=.95)

runs2d_data_raw <- read.table(runs2d_file, header = TRUE, sep = ',')
runs2d_data <- CalculateDataSummary(data=runs2d_data_raw, measurevar="effective_time",
                                  groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                  conf.interval=.95, quantile.interval=.95)
runs2d_data_kernel <- CalculateDataSummary(data=runs2d_data_raw, measurevar="kernel_time",
                                         groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                         conf.interval=.95, quantile.interval=.95)

rank_data <- read.table(rank_file, header = TRUE, sep = ',')
rank_data_compute <- CalculateDataSummary(data=rank_data, measurevar="compute_time",
                                          groupvars=c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores', 'rank'),
                                          conf.interval=.95, quantile.interval=.95)

ggplot(data=ghost_runs_data, aes(x=cores, y=median/1000, group=ghost_cells, color=factor(ghost_cells), linetype=factor(ghost_cells))) +
  scale_color_hue() +
  scale_y_continuous("time [ms]") +
  scale_x_continuous(name="cores", breaks=ghost_runs_data$cores) +
  geom_point() +
  geom_line() +
  labs(title = "Jacobi 1D: median runtime for different halo sizes", subtitle = "problem size scaling with the number of processes", caption = "Euler ETH cluster | EPYC_7H12", color = "halo size", linetype = "halo size")

ggplot(data=runs_data, aes(x=cores, y = median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000)) +
  #geom_hline(yintercept=overall_data_summary[26, "median"], linetype="dashed", color = "red") +
  scale_y_continuous(name="time [ms]") +
  scale_x_continuous(name="processes", breaks=runs_data$cores) +
  geom_point() +
  geom_line(aes(color="total"), size=1.5) +
  geom_line(aes(color="kernel", y=runs_data_kernel$median/1000), size=1) +
  geom_errorbar() +
  labs(title="Jacobi 1D: overall runtimes", subtitle="halo size set to 8, problem size scaling with the number of processes", caption="Euler ETH cluster | EPYC_7H12", y="time [ms]", x="processes", color="measured")

ggplot(data=rank_data_compute, aes(x=rank, y=median/1e6, fill=factor(rank %% 8))) +
  facet_wrap(~cores) +
  scale_fill_brewer(palette = "Greens", guide="none", direction = -1) +
  geom_col() +
  labs(title="Jacobi 1D: compute time per core", subtitle="same job as in the previous graph, excl. sync and write time", caption="Euler ETH cluster | EPYC_7H12", y="median compute time [s]")

ggplot(data=runs2d_data, aes(x=cores, y = median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000)) +
  #geom_hline(yintercept=overall_data_summary[26, "median"], linetype="dashed", color = "red") +
  scale_y_continuous(name="time [ms]") +
  scale_x_continuous(name="processes", breaks=runs2d_data$cores) +
  geom_point() +
  geom_line(aes(color="total"), size=1.5) +
  geom_line(aes(color="kernel", y=runs2d_data_kernel$median/1000), size=1) +
  geom_errorbar() +
  labs(title="Jacobi 2D: overall runtimes", subtitle="halo size set to 8, problem size scaling with the number of processes", caption="Euler ETH cluster | EPYC_7H12", y="time [ms]", x="processes", color="measured")