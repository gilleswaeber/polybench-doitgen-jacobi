library(ggplot2)
library(extrafont)


setwd("D:\\ETH_Work\\Semester_1\\DPHPC\\Project\\Plots")

source("utils.R")
source("stats.R")


local_sum_data <- ReadAllFilesInDir.Aggregate(dir.path="new_results/results_local_sum/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))

local_sum_data_summary <- CalculateDataSummary(data=local_sum_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

caption_plot <- "Min value is :"
caption_plot <- paste(caption_plot, format(min(local_sum_data_summary$median)/1000000, digits=3), "(s)")

local_sum_data_plot <- ggplot(data=local_sum_data_summary, aes(x = factor(threads), y = median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000, group=benchmark)) +
  scale_y_continuous(trans="log10") +
  geom_line(aes(col=benchmark), size=2) +
  geom_point(size=2) +
  geom_errorbar() +
  labs(title="Threads Vs Time (s)", subtitle = "AMD 7H12 | gcc -O3 -mavx2 -mfma",
       y="Time (s)", x="Threads", caption=caption_plot) +
  theme(plot.title=element_text(size=20),
        plot.subtitle=element_text(size=14),
        plot.caption=element_text(size=14),
        text=element_text(family="Tw Cen MT"))

print(local_sum_data_plot)

# library(extrafont)
# library(remotes)
# remotes::install_version("Rttf2pt1", version = "1.3.8")
# extrafont::font_import()
# loadfonts(device = "win")
# windowsFonts()

ggsave(file="local_sum_data_plot.svg", plot=local_sum_data_plot, width = 10, height=10)

scaling_data <- ReadAllFilesInDir.Aggregate(dir.path="new_results/results_scaling/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))

scaling_data_summary <- CalculateDataSummary(data=scaling_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

caption_plot <- "Min value is :"
caption_plot <- paste(caption_plot, format(min(scaling_data_summary$median)/1000000, digits=3), "(s)")

scaling_data_plot <- ggplot(data=scaling_data_summary, aes(x = factor(threads), y = median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000, group=benchmark)) +
  scale_y_continuous(trans="log10") +
  geom_line(aes(col=benchmark), size=2) +
  geom_point(size=2) +
  geom_errorbar() +
  labs(title="Threads Vs Time (s)", subtitle = "AMD 7H12 | gcc -O3 -mavx2 -mfma",
       y="Time (s)", x="Threads", caption=caption_plot) +
  theme(plot.title=element_text(size=20),
        plot.subtitle=element_text(size=14),
        plot.caption=element_text(size=14),
        text=element_text(family="Tw Cen MT"))

print(scaling_data_plot)

# library(extrafont)
# library(remotes)
# remotes::install_version("Rttf2pt1", version = "1.3.8")
# extrafont::font_import()
# loadfonts(device = "win")
# windowsFonts()

ggsave(file="scaling_data_plot.svg", plot=scaling_data_plot, width = 10, height=10)
