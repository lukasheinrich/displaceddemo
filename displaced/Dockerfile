FROM lukasheinrich/generic_madgraphpythiadelphes
WORKDIR /code
RUN curl http://www.fastjet.fr/repo/fastjet-3.1.3.tar.gz | tar -xzvf - && \
    cd fastjet-3.1.3 && ./configure && make -j4  && make install
ADD . /code/displaced
WORKDIR displaced
RUN make
