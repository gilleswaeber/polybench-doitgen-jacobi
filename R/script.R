library(ggplot2)


setwd("D:\\ETH_Work\\Semester_1\\DPHPC\\Project\\R")

source("utils.R")
source("stats.R")
source("aes.R")
#source("lsbpp.R")

#Load data
data.bh_study <-  ReadAllFilesInDir.Aggregate(dir.path="data/", col=c("benchmark", "NR", "NQ", "NP", "threads", "id", "time", "overhead"))


#re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=c("type", #htsize", "memsize", "csize"), conf.interval=.95, quantile.interval=.95)
re.bh_study <- CalculateDataSummary(data=data.bh_study, measurevar="time", groupvars=c("benchmark", "NR", "NQ", "NP", "threads"), conf.interval=.95, quantile.interval=.95)

#This is not useful in our case but might be if we do a more comprehensive banchmark
#For instance we could benchmark the matrix multiply time for each thread
#re.bh_study.toplot <- subset(re.bh_study)

summary.plot.labels <- c("Runtime")
aes.var <- aes(x=threads, y=median/1000000, ymin=CI.NNorm.high/1000000, ymax=CI.NNorm.low/1000000)


thread_16 <- subset(data.bh_study, threads==16)

val <- lsb_shapiro(thread_16, "time")

plot_lat <- ggplot(data=re.bh_study, aes.var) +
  geom_point(size=na_geopoint_size)+  
  geom_line(size=na_geomline_size)+
  geom_errorbar(width = na_geom_errorbar_width, lwd=na_geom_errorbar_width, color="black")+
  scale_color_discrete(name="", labels=summary.plot.labels)+
  scale_shape_discrete(name="",labels=summary.plot.labels)+
  scale_x_continuous(name="Threads")+
  scale_y_continuous(name="Time")+
  theme_bw(na_theme_size) +
  theme(legend.position="bottom", legend.direction = "horizontal", legend.key = element_rect(fill = "transparent", colour = "transparent"), legend.background = element_rect(fill="transparent", colour="transparent")) + 
  theme(plot.margin=na_plot_margin) + theme(text = element_text(size=na_theme_text_size)) + theme(legend.key.height=na_theme_legend_key_height) +
  theme(axis.title.y=element_text(vjust=na_theme_y_vjust)) +
  theme(axis.title.x=element_text(vjust=na_theme_x_vjust))

print(plot_lat)

sink("filename.txt")  # to be found in dir `getwd()`
cat(paste(val[[3]],"\n"))
cat(paste("W : ", val[[1]], "\n"))
cat(paste("p : ", val[[2]], "\n"))
sink()


require(rmarkdown)
my_text <- readLines("filename.txt")
cat(my_text, sep="  \n", file = "markdown_text.Rmd")
render("markdown_text.Rmd", pdf_document())
file.remove("markdown_text.Rmd")

pdf("graph.pdf")
print(plot_lat)
dev.off()

#https://stackoverflow.com/questions/13273611/how-to-append-a-plot-to-an-existing-pdf-file
outFileName <- "Results.pdf"
system2(command = "pdftk",
        args = c("markdown_text.pdf", "graph.pdf", "cat output", shQuote(outFileName)))


#PrintGGPlotOnPDF(plot_lat, "test.pdf")

