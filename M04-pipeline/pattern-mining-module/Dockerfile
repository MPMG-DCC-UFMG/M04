

# FROM gcc:4.9


# RUN gcc /home/Makefile


FROM python:3

WORKDIR /home
COPY / ./

RUN pip install -r requirements.txt


RUN apt-get update \
  && apt-get install -y  \
                        g++ \
                        make \
                        wget

# Download boost, untar, setup install with bootstrap and only do the Program Options library,
# and then install
RUN cd /home && wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz \
  && tar xfz boost_1_60_0.tar.gz \
  && rm boost_1_60_0.tar.gz \
  && cd boost_1_60_0 \
  && ./bootstrap.sh --prefix=/usr/local --with-libraries=program_options dev program-options-dev all-dev \
  && ./b2 install \
  && cd /home \
  && rm -rf boost_1_60_0

RUN cd /home/multidupehack && make
RUN apt-get install bc
RUN apt-get install libboost-all-dev -y
# RUN /home/make