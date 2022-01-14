library(ggplot2)
library(extrafont)
require(plyr)

loadfonts()

setwd("F:\\DPHPC\\Plots")

source("utils.R")
source("stats.R")

blocking_scale_data <- ReadAllFilesInDir.Aggregate(dir.path="results_blocking_scale/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))

blocking_scale_data$benchmark <- mapvalues(blocking_scale_data$benchmark, 
                                     from=c("inverted_loop_blocking"),
                                     to=c("Inverted loop blocking"))

blocking_scale_data_summary <- CalculateDataSummary(data=blocking_scale_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

blocking_scale_data_plot <- ggplot(data=blocking_scale_data_summary, aes(x = factor(blocking_size), y = median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000, group=benchmark)) +
  facet_wrap(~threads, nrow=2) +
  scale_y_continuous(trans="log10", labels = scales::comma) +
  geom_line(aes(col=benchmark), size=1) +
  geom_point(size=1.5) +
  geom_errorbar() +
  labs(subtitle = "Median running time of the blocking alogithm for varying amounts of threads and\nblocking window sizes.",
       caption="NR = 512 | NQ = 2048 | NP = 2048",
       y="Time (ms)", x="Blocking size") +
  theme(plot.title=element_text(size=14),
        plot.caption=element_text(size=8),
        legend.position = c(.9, 0.25))

print(blocking_scale_data_plot)

ggsave(file="blocking_data_scale_plot.svg", plot=blocking_scale_data_plot, width = 8, height=4)
ggsave(file="blocking_data_scale_plot.eps", plot=blocking_scale_data_plot, width = 8, height=4)
