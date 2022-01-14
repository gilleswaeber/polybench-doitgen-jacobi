library(ggplot2)
library(plyr)
library(dplyr)
library(stringr)
library(data.table)
library(extrafont)
library(emojifont)

extrafont::loadfonts()

setwd("D:/Work/ETHZ/hpc/project/")

source("R/utils.R")
source("R/stats.R")

ghost_runs_file <- "report/data/1222155458_1dghost_lsb_runs.csv"
ghost_runs_file <- "report/data/0113132911_1dghost_lsb_runs.csv"
multi2d_file <- "report/data/0111231315_vstack_lsb_runs.csv"
runs2d_file <- "report/data/1223050254_2d_lsb_runs.csv"
j1d_nodes_file <- "report/data/jacobi1d_nodes_m_runs.csv"
j2d_vhalo_file <- "report/data/0113035914_vstackb_lsb_runs.csv"

#Load data
ghost_runs_raw <- read.table(ghost_runs_file, header = TRUE, sep = ',')
ghost_runs_data <- CalculateDataSummary(data = ghost_runs_raw, measurevar = "effective_time",
                                        groupvars = c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                        conf.interval = .95, quantile.interval = .95)

runs2d_data_raw <- read.table(runs2d_file, header = TRUE, sep = ',')
runs2d_data <- CalculateDataSummary(data = runs2d_data_raw, measurevar = "effective_time",
                                    groupvars = c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                    conf.interval = .95, quantile.interval = .95)
runs2d_data_kernel <- CalculateDataSummary(data = runs2d_data_raw, measurevar = "kernel_time",
                                           groupvars = c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                           conf.interval = .95, quantile.interval = .95)

multi2d_raw <- read.csv(multi2d_file)
multi2d_raw$alternative <- mapvalues(multi2d_raw[, "alternative"], from = c(
  "jacobi2d-mpi-benchmark-lsb",
  "jacobi2d-2step-mpi-benchmark-lsb",
  "jacobi2d-vstack-mpi-benchmark-lsb"
), to = c(
  "base",
  "2step",
  "vstack"
))
multi2d_data <- CalculateDataSummary(data = multi2d_raw, measurevar = "effective_time",
                                     groupvars = c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                     conf.interval = .95, quantile.interval = .95)

j1d_nodes_data <- CalculateDataSummary(data= read.csv(j1d_nodes_file), measurevar = "effective_time",
                                       groupvars = c('alternative', 'nodes', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                       conf.interval = .95, quantile.interval = .95)

j2d_vhalo_raw <- read.csv(j2d_vhalo_file)
j2d_vhalo_data <- CalculateDataSummary(data=j2d_vhalo_raw, measurevar = "effective_time",
                                       groupvars = c('alternative', 'n', 'time_steps', 'ghost_cells', 'cores'),
                                       conf.interval = .95, quantile.interval = .95)

base_theme <- theme(plot.title.position = "plot",
                    axis.ticks.y = element_blank(),
                    axis.ticks.x = element_blank(),
                    panel.grid.minor.x = element_blank(),
                    panel.grid.major.x = element_blank(),
                    text = element_text(size = 7),
                    legend.key.size = unit(.7, "line"),
                    legend.spacing.y = unit(.1, "lines"),
                    legend.margin = margin(0, 0, 0, 0, "lines"),
                    panel.spacing = unit(.05, "lines"),
                    plot.margin = unit(c(.1,.05,0,0), "lines")
)

# Jacobi 1D: comparing halo depths
j1d_ghost_runs_plot <- ggplot(data = ghost_runs_data, aes(x = cores, y = median / 1000000, group = ghost_cells, color = factor(ghost_cells))) +
  #scale_color_brewer(palette="RdYlBu") +
  scale_color_manual(values=c('#D73027','#F46D43','#FDAE61','#EED000','#FFFF80','#A0D0D0','#ABD9E9','#74ADD1','#4575B4')) +
  scale_y_continuous(NULL, limits = c(0, NA)) +
  scale_x_continuous(name = "#cores", breaks = ghost_runs_data$cores) +
  scale_shape_manual(values = c(15, 16, 17, 21, 22, 23, 3, 8, 4)) +
  geom_line(aes(), size = 0.4) +
  geom_point(aes(shape = factor(ghost_cells)), size = 1) +
  labs(subtitle = "median time [s] | single node | N=1 000 000 S, T=1 000", color = "halo depth", linetype = "halo depth", shape = "halo depth") +
  base_theme +
  theme(legend.position = c(.88, .190), legend.key.size = unit(.5, "line"), legend.margin = margin(.07, .15, .07, .1, "lines")) +
  guides(color=guide_legend(ncol=4))
#j1d_ghost_runs_plot
ggsave("report/jacobi_mpi/1d_halo.pdf", plot = j1d_ghost_runs_plot, width = 3.287, height = 1.5, unit = "in")

# Jacobi 1D: spanning multiple nodes
j1d_nodes_plot <-
ggplot(data = j1d_nodes_data, aes(x = factor(nodes), y = median / 1000000, ymin=CI.NNorm.low/1000000, ymax=CI.NNorm.high/1000000, group=nodes, fill=factor(nodes))) +
  facet_wrap(~cores, nrow = 1) +
  #scale_fill_brewer(direction = -1) +
  scale_y_continuous(NULL, limits = c(0, NA)) +
  scale_x_discrete(NULL, labels = NULL) +
  geom_bar(stat="identity") +
  geom_errorbar(size=.2) +
  base_theme +
  theme(legend.position = c(.077, .665), legend.key.size = unit(.5, "line"), legend.margin = margin(.1, .2, .1, .2, "lines")) +
  labs(subtitle = "time [s] by #cores, #nodes | depth = 8 | N = 1 000 000 S, T = 1 000", fill = "#nodes") +
  guides(fill=guide_legend(ncol=2))
ggsave("report/jacobi_mpi/1d_nodes.pdf", plot = j1d_nodes_plot, width = 3.287, height = 1.4, unit = "in")

# Jacobi 2D: program variants
multi2d_plot <- ggplot(data = multi2d_data, aes(interaction(ghost_cells, alternative), y=median / 1000000, ymin=CI.NNorm.low/1000000, ymax=CI.NNorm.high/1000000, fill = alternative)) +
  facet_wrap(~cores, labeller = labeller(
    cores = function (x) ifelse (x == 1, "1 core", paste0(x, " cores"))
  ), nrow = 1) +
  scale_y_continuous(NULL, limits = c(0, NA)) +
  scale_x_discrete("halo depths: 1 8 16 32 64", labels = NULL) +  # function (x) stringr::str_extract(x, "\\d+")
  geom_bar(size = .4, stat = "identity") +
  #scale_fill_discrete(name="Variant", breaks = c("jacobi2d-mpi-benchmark", "jacobi2d-2step-mpi-benchmark", "jacobi2d-vstack-mpi-benchmark"), labels = c('A', 'B', 'C')) +
  base_theme +
  geom_errorbar(size=.2) +
  theme(legend.position = c(.08, .74), legend.title = element_blank(), legend.key.size = unit(.5, "line"), legend.margin = margin(.1, .8, .1, .1, "lines"), legend.spacing.y = unit(0, "mm")) +
  labs(subtitle="time [s] | single node | N=1 000 sqrt(S), T=1 000", fill = "Variant")
#multi2d_plot
ggsave("report/jacobi_mpi/2d_multi.pdf", plot = multi2d_plot, width = 3.287, height = 1.25, unit = "in")

# Jacobi 2D: halo depth for vstack
j2d_vhalo_plot <-
  ggplot(data = j2d_vhalo_data, aes(x = factor(ghost_cells), y = median / 1000000, ymin=CI.NNorm.low/1000000, ymax=CI.NNorm.high/1000000, group=ghost_cells, fill=factor(ghost_cells))) +
    facet_wrap(~cores, labeller = labeller(
      cores = function (x) ifelse (x == 1, "1 core", paste0(x, " cores"))
    ), nrow = 1) +
    #scale_fill_brewer(direction = -1) +
    scale_y_continuous(NULL, limits = c(0, NA)) +
    scale_x_discrete(NULL, labels = NULL) +
    geom_bar(stat="identity") +
    geom_errorbar(size=.3) +
    base_theme +
    theme(legend.position = c(.083, .7), legend.key.size = unit(.5, "line"), legend.margin = margin(.1, .25, .1, 0, "lines")) +
    labs(subtitle = "time [s] by #cores, depth | single node, vstack | N = 1 000 sqrt(S), T = 1 000", fill = "halo depth") +
    guides(fill=guide_legend(ncol=2))
ggsave("report/jacobi_mpi/2d_vhalo.pdf", plot = j2d_vhalo_plot, width = 3.287, height = 1.2, unit = "in")

ggplot(data = runs2d_data, aes(x = cores, y = median / 1000, ymin = CI.NNorm.high / 1000, ymax = CI.NNorm.low / 1000)) +
  #geom_hline(yintercept=overall_data_summary[26, "median"], linetype="dashed", color = "red") +
  scale_y_continuous(name = "time [ms]", limits = c(0, NA)) +
  scale_x_continuous(name = "processes", breaks = runs2d_data$cores) +
  geom_point() +
  geom_line(aes(color = "total"), size = 1.5) +
  geom_line(aes(color = "kernel", y = runs2d_data_kernel$median / 1000), size = 1) +
  geom_errorbar() +
  labs(title = "Jacobi 2D: overall runtimes", subtitle = "halo size set to 8, problem size scaling with the number of processes", caption = "Euler ETH cluster | EPYC_7H12", y = "time [ms]", x = "processes", color = "measured")