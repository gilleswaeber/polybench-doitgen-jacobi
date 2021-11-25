library(ggplot2)


setwd("D:\\ETH_Work\\Semester_1\\DPHPC\\Project\\R")

source("utils.R")
source("stats.R")
source("aes.R")
#source("lsbpp.R")

#Load data
data.bh_study <-  ReadAllFilesInDir.Aggregate(dir.path="new_data/", col=c("NR", "NQ", "NP", "benchmark", "threads", "blocking_size", "id", "time", "overhead"))

re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=c("NR", "NQ", "NP", "benchmark", "threads"), conf.interval=.95, quantile.interval=.95)

seq_constant <- re.bh_study[re.bh_study$benchmark == "doitgen-seq", 9] 
line.data <- data.frame(yintercept = c(seq_constant / 1000), Lines = c("Polybench  doitgen sequential"))

#Comparisons of algorithms
gg <- ggplot(re.bh_study, aes(x=threads, y=median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000)) +
  geom_point(aes(col=benchmark, shape=benchmark), size=3) +
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black") +
  geom_line(aes(col=benchmark), size = 1) +
  scale_colour_brewer(palette = "Dark2") +
  labs(title = "Threads vs Time", subtitle = "Euler cluster : 48 cores\nNR = 128; NQ = 512; NP = 512", x="Threads", y="Time (ms)") +
  #scale_x_continuous(breaks=seq(1, 16, 1)) +
  scale_x_continuous(breaks=re.bh_study$threads, trans="log2") +
  scale_y_continuous(trans="log10") +
  geom_hline(aes(yintercept=yintercept, linetype = Lines), line.data, size=2, col="red")


gg + theme(plot.title =element_text(size=20,
                         face="bold"))




aes.var <- aes(x=threads, y=median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000)
plot_lat <- ggplot(data=re.bh_study, aes.var) +
  geom_point(size=na_geopoint_size, aes(col=benchmark))+  
  geom_line(size=na_geomline_size, aes(col=benchmark))+
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black")+
  scale_x_continuous(name="Threads", breaks=re.bh_study$threads, trans="log2")+
  scale_y_continuous(name="Time", trans="log10")+
  theme_bw(na_theme_size) +
  theme(legend.position="bottom", legend.direction = "horizontal", legend.key = element_rect(fill = "transparent", colour = "transparent"), legend.background = element_rect(fill="transparent", colour="transparent")) + 
  theme(plot.margin=na_plot_margin) + theme(text = element_text(size=na_theme_text_size)) + theme(legend.key.height=na_theme_legend_key_height) +
  theme(axis.title.y=element_text(vjust=na_theme_y_vjust)) +
  theme(axis.title.x=element_text(vjust=na_theme_x_vjust))

print(plot_lat)

#Blocking graph
blocking_data <- subset(data.bh_study, blocking_size!="NA")
blocking_data_summary <- CalculateDataSummary(data=blocking_data, measurevar="time", groupvars=c("NR", "NQ", "NP", "benchmark", "threads", "blocking_size"), conf.interval=.95, quantile.interval=.95)

gg <- ggplot(blocking_data_summary, aes(x=blocking_size, y=median/1000, ymin=CI.NNorm.high/1000, ymax=CI.NNorm.low/1000)) +
  geom_point(size=3) +
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black") +
  geom_line(size = 1) +
  labs(title = "Blocking size vs Time", subtitle = "Euler cluster : 48 cores\nNR = 128; NQ = 512; NP = 512", x="Blocking size", y="Time (ms)") +
  #scale_x_continuous(breaks=seq(1, 16, 1)) +
  scale_x_continuous(breaks=re.bh_study$threads, trans="log2") +
  scale_y_continuous(trans="log10")


gg + theme(plot.title =element_text(size=20,
                                    face="bold"))
