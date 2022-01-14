library(ggplot2)
library(extrafont)
require(plyr)

loadfonts()

setwd("F:\\DPHPC\\Plots")

source("utils.R")
source("stats.R")

classic_data_1 <- ReadAllFilesInDir.Aggregate(dir.path="results_classic/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))
sequential_data_2 <- ReadAllFilesInDir.Aggregate(dir.path="results_sequential_512/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))



classic_data <- rbind(classic_data_1, sequential_data_2)


classic_data$benchmark <- mapvalues(classic_data$benchmark, 
                                     from=c("inverted_loop", "inverted_loop_avx2",
                                            "inverted_loop_avx2_local_sum",
                                            "inverted_loop_avx2_local_sum_1D",
                                            "inverted_loop_local_sum",
                                            "inverted_loop_local_sum_1D",
                                            "transpose_local_sum",
                                            "polybench",
                                            "polybench_parallel_local_sum",
                                            "transpose"),
                                     to=c("Inverted loop", "Inverted loop AVX2",
                                          "Inverted loop AVX2 local sum 2D",
                                          "Inverted loop AVX2 local sum 1D",
                                          "Inverted loop local sum 2D",
                                          "Inverted loop local sum 1D",
                                          "Transpose local sum 1D",
                                          "Polybench",
                                          "Polybench parallel local sum 1D",
                                          "Transpose"))

classic_data_summary <- CalculateDataSummary(data=classic_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

for (row_nb in c(43)) {
  for(thread_nb in c(2, 4, 8, 16, 32, 48)) {
    new_row <- classic_data_summary[row_nb,]
    new_row$threads = thread_nb
    classic_data_summary <- rbind(classic_data_summary, new_row)
  }
}

min_val_classic_data <- min(classic_data_summary$median) / 1000
line_label_classic_data <-  paste(format(min_val_classic_data, digits=3), "ms")

classic_data_plot <- ggplot(data=classic_data_summary, aes(x = factor(threads), y=median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000, group=benchmark)) +
    scale_y_continuous(labels = scales::comma, trans="log10") +
    geom_line(aes(col=benchmark), size=1) +
    geom_point(size=1.5) +
    geom_hline(yintercept=min_val_classic_data, linetype="dashed", color="red", size=1) +
    #geom_text(aes(0, min_val_classic_data, label = line_label_classic_data, vjust=-1, hjust = -0.5)) +
    annotate("text", x = 0, y = min_val_classic_data, label = format(min_val_classic_data, digits=3), hjust = 1.2, vjust=1.7, color="red") + coord_cartesian(clip = 'off') +
    labs(subtitle = "Median running time of the implmentation depending on the thread count.",
         caption = "NR = 512 | NQ = 512 | NP = 512",
         y="Time (ms)", x="Threads") +
    theme(plot.title=element_text(size=14),
          plot.caption=element_text(size=8))

print(classic_data_plot)

ggsave(file="classic_data_plot.svg", plot=classic_data_plot, width = 6, height=3.5)
ggsave(file="classic_data_plot.eps", plot=classic_data_plot, width = 6, height=3.5)

classic_data_boxplot <- ggplot(data=classic_data, aes(x = factor(threads), y=time/1000)) +
  facet_wrap(~benchmark) +
  scale_y_continuous(labels = scales::comma) +
  geom_boxplot() +
  labs(title="Threads Vs Time (ms)", subtitle = "AMD 7H12 | gcc(9.3.0) -O3 -mavx2 -mfma | NR = 512; NQ = 512; NP = 512",
       y="Time (ms)", x="Threads") +
  theme(plot.title=element_text(size=14),
        plot.caption=element_text(size=8))
  
  
print(classic_data_boxplot)

ggsave(file="classic_data_boxplot.svg", plot=classic_data_boxplot, width = 6, height = 5)
ggsave(file="classic_data_boxplot.eps", plot=classic_data_boxplot, width = 10, height=10)
