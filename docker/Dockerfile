FROM amazonlinux:2

RUN yum -y update
RUN yum install -y vim git gcc gcc-c++ openssl-devel sudo passwd tar make unzip procps autoconf
RUN amazon-linux-extras install php7.2
RUN yum install -y php-devel

RUN echo 'root:root' | chpasswd
RUN useradd -ms /bin/bash ec2-user
RUN passwd -d ec2-user
RUN echo 'ec2-user ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers.d/ec2-user
RUN chmod 0440 /etc/sudoers.d/ec2-user

RUN echo "NETWORKING=yes" > /etc/sysconfig/network

COPY kakasi-2.3.6.tar.gz /
RUN tar zxf kakasi-2.3.6.tar.gz && cd kakasi-2.3.6 && ./configure && make && make install && cd ../
RUN git clone https://github.com/helloooideeeeea/php-kakasi.git && cd php-kakasi/kakasi && phpize && autoconf -f && ./configure && make && make install && cd ../
ADD add_ld.conf /etc/ld.so.conf.d
ADD 40-kakasi.ini /etc/php.d
RUN ldconfig

RUN mkdir /home/ec2-user/develop

USER ec2-user
WORKDIR /home/ec2-user

#RUN cd /home/ec2-user/develop
#RUN git clone https://github.com/php/php-src.git && cd php-src
#RUN git checkout php-7.2.9

