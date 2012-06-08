#include "sharedbool.h"

namespace iom
{

    SharedBool::SharedBool(bool val)
    :val(val) {
    }

    void SharedBool::set(bool newval) {
        boost::lock_guard<boost::mutex> lock(mut);
        val = newval;
        cond.notify_all();
    }

    bool SharedBool::wait(bool waited) {
        boost::unique_lock<boost::mutex> lock(mut);
        while (val != waited) {
            cond.wait(lock);
        }
        return val == waited;
    }
}