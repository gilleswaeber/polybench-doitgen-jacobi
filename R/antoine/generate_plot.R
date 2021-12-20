library(ggplot2)


setwd("D:\\ETH_Work\\Semester_1\\DPHPC\\Project\\Plots")

source("utils.R")
source("stats.R")

#line.data <- data.frame(yintercept = c(seq_constant / 1000), Lines = c("Polybench  doitgen sequential"))

fullnode_data <- ReadAllFilesInDir.Aggregate(dir.path="results_7H12_fullnode_no_scaling/", col=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size", "id", "time", "overhead"))

blocking_data <- subset(fullnode_data, blocking_size != 0)

blocking_data_summary <- CalculateDataSummary(data=blocking_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

caption_blocking_plot <- "Min value is :"
caption_blocking_plot <- paste(caption_blocking_plot, format(min(blocking_data_summary$median)/1000000, digits=3), "(s)")

blocking_data_plot <- ggplot(data=blocking_data_summary, aes(x = factor(blocking_size), y = median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000, group=benchmark)) +
  facet_wrap(~threads) +
  scale_y_continuous(trans="log10") +
  geom_line(aes(col=benchmark), size=2) +
  geom_point(size=2) +
  geom_errorbar() +
  labs(title="Blocking size Vs Time (s)", subtitle = "AMD 7H12 | gcc -O3 -mavx2 -mfma",
       y="Time (s)", x="Blocking size", caption=caption_blocking_plot) +
  theme(plot.title=element_text(size=20),
        plot.subtitle=element_text(size=14),
        plot.caption=element_text(size=14),
        text=element_text(family="Tw Cen MT"))

print(blocking_data_plot)

# library(extrafont)
# library(remotes)
# remotes::install_version("Rttf2pt1", version = "1.3.8")
# extrafont::font_import()
# loadfonts(device = "win")
# windowsFonts()

ggsave(file="blocking_plot.svg", plot=blocking_data_plot, width = 10, height=10)

fullnode_no_blocking_data <- subset(fullnode_data, blocking_size == 0)

fullnode_no_blocking_data_summary <- CalculateDataSummary(data=fullnode_no_blocking_data, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

seq_constant <- fullnode_no_blocking_data_summary[fullnode_no_blocking_data_summary$benchmark == "polybench", 11]
line.data <- data.frame(yintercept = c(seq_constant / 1000000), Lines = c("Polybench"))

caption_plot <- "Min value is :"
caption_plot <- paste(caption_plot, format(min(fullnode_no_blocking_data_summary$median)/1000000, digits=3), "(s)")

fullnode_no_blocking_data_plot <- ggplot(data=fullnode_no_blocking_data_summary, aes(x = factor(threads), y=median/1000000, group=benchmark)) +
    scale_y_continuous(trans="log10") +
    geom_line(aes(col=benchmark), size=2) +
    geom_point(size=2) +
  geom_hline(aes(yintercept=yintercept, linetype = Lines), line.data, size=2, col="red") +
  #geom_line(data = line.data, color="red") +
    labs(title="Threads Vs Time (s)", subtitle = "AMD 7H12 | gcc -O3 -mavx2 -mfma",
         y="Time (s)", x="Threads", caption=caption_plot) +
    theme(plot.title=element_text(size=20),
          plot.subtitle=element_text(size=14),
          plot.caption=element_text(size=14),
          text=element_text(family="Tw Cen MT"))
print(fullnode_no_blocking_data_plot)

ggsave(file="fullnode_no_blocking_data_plot.svg", plot=fullnode_no_blocking_data_plot, width = 10, height=10)

fullnode_no_blocking_data_boxplot <- ggplot(data=fullnode_no_blocking_data, aes(x = factor(threads), y=time/1000000)) +
  facet_wrap(~benchmark) +
  scale_y_continuous(trans="log10") +
  geom_boxplot() +
  labs(title="Threads Vs Time (s)", subtitle = "AMD 7H12 | gcc -O3 -mavx2 -mfma",
       y="Time (s)", x="Threads") +
  theme(plot.title=element_text(size=20),
        plot.subtitle=element_text(size=14),
        plot.caption=element_text(size=14),
        text=element_text(family="Tw Cen MT"))
  
  
print(fullnode_no_blocking_data_boxplot)

ggsave(file="fullnode_no_blocking_data_boxplot.svg", plot=fullnode_no_blocking_data_boxplot, width = 10, height=10)





bsub_data <- ReadAllFilesInDir.Aggregate(dir.path="bsub_benchmark/", col=c("TABLE_SIZE", "id", "time", "overhead"))
res <- bsub_data$time / bsub_data$TABLE_SIZE

bsub_data$access_time <- res


ggplot(data=bsub_data, aes(x = factor(TABLE_SIZE), y=access_time)) +
  geom_point(size=2)
