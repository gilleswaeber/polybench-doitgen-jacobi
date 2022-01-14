library(ggplot2)
library(extrafont)
library(plyr)

setwd("F:\\DPHPC\\Plots")

source("utils.R")
source("stats.R")

scaling_data <- ReadAllFilesInDir.Aggregate(dir.path="results_scaling_final_2/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))

scaling_data$benchmark <- mapvalues(scaling_data$benchmark, 
                                    from=c("inverted_loop_avx2_blocking"),
                                    to=c("Inverted loop AVX2 blocking"))

scaling_data_summary <- CalculateDataSummary(data=scaling_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

scaling_data_plot <- ggplot(data=scaling_data_summary, aes(x = factor(threads), y = median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000, group=benchmark)) +
  scale_y_continuous(labels = scales::comma) +
  geom_line(aes(col=benchmark), size=1) +
  geom_point(size=1.5) +
  geom_errorbar() + 
  labs(subtitle = "Median running time of the kernel depending on the thread count.\nNR is scaled by the number of threads.", caption = "NR = 128*#Threads | NQ = 2048 | NP = 2048 | Windows size = 32",
       y="Time (ms)", x="Threads") +
  theme(plot.title=element_text(size=14),
        plot.caption=element_text(size=8))

print(scaling_data_plot)

ggsave(file="scaling_data_plot.svg", plot=scaling_data_plot, width = 5.5, height=2)
ggsave(file="scaling_data_plot.eps", plot=scaling_data_plot, width = 5.5, height=2)
